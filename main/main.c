#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "driver/gpio.h"

#include "esp_log.h"
#include "nvs_flash.h"

#include "include/wifi.h"
#include "include/control_server.h"
#include "include/hw_controller.h"

char *TAG = "main";

void control_event_handler(void* handler_arg, esp_event_base_t base, int32_t id, void* event_data) {
    switch (id) {
    case CONTROL_LIGHT_EVENT:
        control_light_event_t *light_event = (control_light_event_t*) event_data;
        ESP_LOGI(TAG, "Received light event headlight: %d, taillight: %d, blinker: %d", light_event->headlight, light_event->taillight, light_event->blinker);
        set_headlight_mode(light_event->headlight);
        set_taillight_mode(light_event->taillight);
        set_blinker_mode(light_event->blinker);
        break;
    case CONTROL_MOTOR_EVENT:
        control_motor_event_t *motor_event = (control_motor_event_t*) event_data;
        ESP_LOGI(TAG, "Received motor event duty cycle: %d", motor_event->duty_cycle);
        set_motor_duty(motor_event->duty_cycle);
        break;
    default:
        break;
    }
}

void app_main(void) {
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

    // Create UDP Socket and start listening
    ESP_LOGI(TAG, "Starting UDP Server");
    init_control_server();
    register_control_server_event_handler(control_event_handler);
    start_control_server();

    init_hw_controller();
}
