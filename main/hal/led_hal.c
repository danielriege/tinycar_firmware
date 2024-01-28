#include "led_hal.h"

ledc_timer_config_t ledc_timer;
ledc_channel_config_t ledc_headlight_config;
ledc_channel_config_t ledc_taillight_config;

uint8_t blinker_mode = 0;
uint8_t next_state = 0;

void blinker_task() {
    while (1) {
        if (blinker_mode == 1) {
            gpio_set_level(PIN_BLINKER_LEFT, next_state);
            gpio_set_level(PIN_BLINKER_RIGHT, 0);
        } else if (blinker_mode == 2) {
            gpio_set_level(PIN_BLINKER_LEFT, 0);
            gpio_set_level(PIN_BLINKER_RIGHT, next_state);
        } else if (blinker_mode == 3) {
            gpio_set_level(PIN_BLINKER_LEFT, next_state);
            gpio_set_level(PIN_BLINKER_RIGHT, next_state);
        } else {
            gpio_set_level(PIN_BLINKER_LEFT, 0);
            gpio_set_level(PIN_BLINKER_RIGHT, 0);
        }
        if (next_state == 0) {
            next_state = 1;
        } else {
            next_state = 0;
        }
        vTaskDelay(BLINKER_INTERVAL);
    }
}

void init_led_controller() {
    // change IO mux for Blinker Pins
    gpio_iomux_out(PIN_BLINKER_RIGHT, FUNC_MTCK_GPIO39, false);
    gpio_iomux_out(PIN_BLINKER_LEFT, FUNC_MTDO_GPIO40, false);
    // GPIO
    if (gpio_set_direction(PIN_BLINKER_LEFT, GPIO_MODE_OUTPUT) != ESP_OK) {
        ESP_LOGE("led_hal", "Error setting direction for blinker left");
    }
    if (gpio_set_direction(PIN_BLINKER_RIGHT, GPIO_MODE_OUTPUT) != ESP_OK) {
        ESP_LOGE("led_hal", "Error setting direction for blinker right");
    }
    // PWM
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .timer_num        = LEDC_TIMER,
        .duty_resolution  = LEDC_DUTY_RES,
        .freq_hz          = LEDC_FREQUENCY,  // Set output frequency at 5 kHz
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));
    
    ledc_headlight_config = (ledc_channel_config_t){
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_HEADLIGHT_CHANNEL,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = PIN_HEADLIGHT,
        .duty           = 0, // Set duty to 0%
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_headlight_config));

    ledc_taillight_config = (ledc_channel_config_t){
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_TAILLIGHT_CHANNEL,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = PIN_TAILLIGHT,
        .duty           = 0, // Set duty to 0%
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_taillight_config));

    //gpio_dump_io_configuration(stdout, BIT64(PIN_BLINKER_LEFT) | BIT64(PIN_BLINKER_RIGHT) | BIT64(PIN_HEADLIGHT) | BIT64(PIN_TAILLIGHT));
    
    xTaskCreate(&blinker_task, "blinker_task", 2048, NULL, 6, NULL);
}

void led_set_headlight_mode(uint8_t mode) {
    if (mode == 1) {
        ledc_set_duty(LEDC_MODE, LEDC_HEADLIGHT_CHANNEL, LEDC_DUTY_25);
        ledc_update_duty(LEDC_MODE, LEDC_HEADLIGHT_CHANNEL);
    } else if (mode == 2) {
        ledc_set_duty(LEDC_MODE, LEDC_HEADLIGHT_CHANNEL, LEDC_DUTY_100);
        ledc_update_duty(LEDC_MODE, LEDC_HEADLIGHT_CHANNEL);
    } else {
        ledc_set_duty(LEDC_MODE, LEDC_HEADLIGHT_CHANNEL, 0);
        ledc_update_duty(LEDC_MODE, LEDC_HEADLIGHT_CHANNEL);
    }
}

void led_set_taillight_mode(uint8_t mode) {
    if (mode == 1) {
        ledc_set_duty(LEDC_MODE, LEDC_TAILLIGHT_CHANNEL, LEDC_DUTY_75);
        ledc_update_duty(LEDC_MODE, LEDC_TAILLIGHT_CHANNEL);
    } else if (mode == 2) {
        ledc_set_duty(LEDC_MODE, LEDC_TAILLIGHT_CHANNEL, LEDC_DUTY_100);
        ledc_update_duty(LEDC_MODE, LEDC_TAILLIGHT_CHANNEL);
    } else {
        ledc_set_duty(LEDC_MODE, LEDC_TAILLIGHT_CHANNEL, 0);
        ledc_update_duty(LEDC_MODE, LEDC_TAILLIGHT_CHANNEL);
    }
}

void led_set_blinker_mode(uint8_t mode) {
    blinker_mode = mode;
}