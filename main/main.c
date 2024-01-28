#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "driver/gpio.h"
#include "driver/usb_serial_jtag.h"
#include "esp_vfs_usb_serial_jtag.h"
#include "esp_vfs_dev.h"
#include "sdkconfig.h"

#include "esp_log.h"
#include "nvs_flash.h"

#include "wifi.h"
#include "storage.h"
#include "protocol/tccp.h"
#include "protocol/rtp.h"

#include "hal/led_hal.h"
#include "hal/camera_hal.h"
#include "hal/battery_status_hal.h"
#include "hal/servo_hal.h"
#include "hal/motor_hal.h"

#include "congestion_control.h"

char *TAG = "main";

static congestion_control_t congestion_control_params;

// event handler from UDP server
void control_event_handler(void* handler_arg, esp_event_base_t base, int32_t id, void* event_data) {
    switch (id) {
    case TCCP_CONTROL_EVENT: ;
        tccp_control_t *control = (tccp_control_t*) event_data;
        ESP_LOGI(TAG, "Received control event headlight: %d, taillight: %d, blinker: %d, servo angle: %d, motor duty cycle: %d", control->headlight, control->taillight, control->blinker, control->servo_angle, control->motor_duty_cycle);

        #ifdef CONFIG_LIGHT_CONTROL
        led_set_headlight_mode(control->headlight);
        led_set_taillight_mode(control->taillight);
        led_set_blinker_mode(control->blinker);
        #endif

        servo_set_angle(control->servo_angle);
        motor_set_duty_cycle(control->motor_duty_cycle);
        // set destination for rtp
        in_addr_t* dest_ip = get_address_ref();
        #ifdef CONFIG_CAMERA_ENABLED
        rtp_set_destination(dest_ip);
        #endif
        break;
    default:
        break;
    }
}

void send_telemetry_task(void *pvParameters) {
    while (1) {
        int battery_voltage = battery_status_read();
        uint8_t current_fps = 1000 / congestion_control_params.tranmission_interval;
        uint16_t min_frame_latency = rtp_get_last_packet_count() * congestion_control_params.packet_delay;
        int8_t wifi_rssi = wifi_read_rssi();
        send_telemetry(battery_voltage, current_fps, min_frame_latency, wifi_rssi);
        ESP_LOGI(TAG, "telemetry: battery_voltage: %d, current_fps: %d, min_frame_latency: %d, wifi_rssi: %d", battery_voltage, current_fps, min_frame_latency, wifi_rssi);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

void send_camera_frame_task(void *pvParameters) {
    while (1) {
        rtp_set_packet_delay(congestion_control_params.packet_delay);
        camera_fb_t* fb = camera_get_frame_buffer();
        rtp_send_frame(fb->buf, fb->len, fb->width, fb->height);
        camera_return_frame_buffer(fb);
        vTaskDelay(congestion_control_params.tranmission_interval / portTICK_PERIOD_MS);
    }
}

void configure_stdin_stdout(void) {
    static bool configured = false;
    if (configured) {
      return;
    }
    /* Disable buffering on stdin */
    setvbuf(stdin, NULL, _IONBF, 0);

    /* Minicom, screen, idf_monitor send CR when ENTER key is pressed */
    esp_vfs_dev_usb_serial_jtag_set_rx_line_endings(ESP_LINE_ENDINGS_CR);
    /* Move the caret to the beginning of the next line on '\n' */
    esp_vfs_dev_usb_serial_jtag_set_tx_line_endings(ESP_LINE_ENDINGS_CRLF);

    /* Enable non-blocking mode on stdin and stdout */
    fcntl(fileno(stdout), F_SETFL, 0);
    fcntl(fileno(stdin), F_SETFL, 0);

    usb_serial_jtag_driver_config_t usb_serial_jtag_config;
    usb_serial_jtag_config.rx_buffer_size = 1024;
    usb_serial_jtag_config.tx_buffer_size = 1024;

    esp_err_t ret = ESP_OK;
    /* Install USB-SERIAL-JTAG driver for interrupt-driven reads and writes */
    ret = usb_serial_jtag_driver_install(&usb_serial_jtag_config);
    if (ret != ESP_OK) {
        return;
    }

    /* Tell vfs to use usb-serial-jtag driver */
    esp_vfs_usb_serial_jtag_use_driver();
}

void app_main(void) {
    printf("portTICK_PERIOD_MS: %ld\n", portTICK_PERIOD_MS);
    //Initialize NVS
    storage_init();

    // Configure stdin/stdout so we can use std::cout/cin
    configure_stdin_stdout();

    // Initialize WiFi Connection
    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    // read ssid and password from NVS
    char **ssid = malloc(sizeof(char*));
    char **password = malloc(sizeof(char*));
    storage_read_wifi_credentials(ssid, password);
    if (*ssid == NULL || *password == NULL) {
        ESP_LOGI(TAG, "Wifi credentials not found in NVS. Using Config from menuconfig.");
        // Wifi credentials not found in NVS. Use CONFIG_ESP_WIFI_SSID and CONFIG_ESP_WIFI_PASSWORD.
        *ssid = CONFIG_ESP_WIFI_SSID;
        *password = CONFIG_ESP_WIFI_PASSWORD;
    }
    if (wifi_init_sta(*ssid, *password) < 0) {
        // Wifi connect to AP failed. Use Serial console to configure Wifi and save config to NVS.
        printf("Please enter SSID: \n");
        char ssid[32];
        if (fgets(ssid, 32, stdin) == NULL) {
            ESP_LOGE(TAG, "Failed to get SSID");
        } else {
            ssid[strcspn(ssid, "\n")] = '\0';
        }

        printf("SSID: %s\n", ssid);

        printf("Please enter password: \n");
        char password[64];
        if (fgets(password, 64, stdin) == NULL) {
            ESP_LOGE(TAG, "Failed to get SSID");
        } else {
            password[strcspn(password, "\n")] = '\0';
        }
        printf("Password: %s\n", password);
        // store ssid and password in NVS
        storage_write_wifi_credentials(ssid, password);
        ESP_LOGI(TAG, "Wifi credentials saved to NVS. Please restart the device.");
        return;
    }

    // Initialize LED controller
    #ifdef CONFIG_LIGHT_CONTROL
    init_led_controller();
    #endif

    // init servo
    #ifdef CONFIG_SERVO_CONTROL
    init_servo();
    #endif

    // init motor
    init_motor();

    // Start camera setup
    congestion_control_params = congestion_control_init();
    #ifdef CONFIG_CAMERA_ENABLED
    camera_init();
    //xTaskCreate(send_camera_frame_task, "send_camera_frame_task", 4096, NULL, 5, NULL);
    #endif

    // Start battery status setup
    #ifdef CONFIG_BATTERY_STATUS_ENABLED
    battery_status_init();
    #endif

    // Create UDP Socket and start listening
    ESP_LOGI(TAG, "Starting TCCP");
    init_tccp();
    register_tccp_event_handler(control_event_handler);
    start_tccp();

    // create a task to send telemetry periodically
    //#ifdef CONFIG_TELEMETRY_ENABLED
    xTaskCreate(send_telemetry_task, "send_telemetry_task", 4096, NULL, 5, NULL);
    //#endif
}
