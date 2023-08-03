#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "driver/gpio.h"

#include "esp_log.h"
#include "nvs_flash.h"

#include "include/wifi.h"
#include "include/control_server.h"

#define HEADLIGHT_GPIO 3
#define TAILLIGHT_GPIO 5
#define BLINKER_LEFT_GPIO 15
#define BLINKER_RIGHT_GPIO 7
#define MOTOR_GPIO 18

uint8_t blinker_state = 0;
uint8_t blinker_mode = 0;

char *TAG = "main";

void init_hw(void)
{
    gpio_set_direction(BLINKER_LEFT_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_direction(BLINKER_RIGHT_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_direction(HEADLIGHT_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_direction(TAILLIGHT_GPIO, GPIO_MODE_OUTPUT);
}

void blinker_task() {
    while (1) {
        if (blinker_mode == 0) {
            gpio_set_level(BLINKER_LEFT_GPIO, 0);
            gpio_set_level(BLINKER_RIGHT_GPIO, 0);
        } else {
            blinker_state = !blinker_state;
            if (blinker_mode == 1 || blinker_mode == 3) {
                gpio_set_level(BLINKER_LEFT_GPIO, blinker_state);
            } else {
                 gpio_set_level(BLINKER_LEFT_GPIO, 0);
            }
            if (blinker_mode == 2 || blinker_mode == 3) {
                gpio_set_level(BLINKER_RIGHT_GPIO, blinker_state);
            } else {
                gpio_set_level(BLINKER_RIGHT_GPIO, 0);
            }
        }
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

void control_event_handler(void* handler_arg, esp_event_base_t base, int32_t id, void* event_data) {
    switch (id) {
    case CONTROL_LIGHT_EVENT:
        control_light_event_t *light_event = (control_light_event_t*) event_data;
        ESP_LOGI(TAG, "Received light event headlight: %d, taillight: %d, blinker: %d", light_event->headlight, light_event->taillight, light_event->blinker);
        blinker_mode = light_event->blinker;
        blinker_state = 0;
        
        gpio_set_level(HEADLIGHT_GPIO, light_event->headlight);
        gpio_set_level(TAILLIGHT_GPIO, light_event->taillight);
        break;
    case CONTROL_MOTOR_EVENT:
        control_motor_event_t *motor_event = (control_motor_event_t*) event_data;
        ESP_LOGI(TAG, "Received motor event motor_voltage: %d", motor_event->motor_voltage);
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

    init_hw();
    xTaskCreate(&blinker_task, "blinker_task", 2048, NULL, 5, NULL);
}
