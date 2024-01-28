#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "driver/gpio.h"

#include "esp_log.h"
#include "nvs_flash.h"

#include "wifi.h"
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

void app_main(void) {
    printf("portTICK_PERIOD_MS: %ld\n", portTICK_PERIOD_MS);
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize WiFi Connection
    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    wifi_init_sta();

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
