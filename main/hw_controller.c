#include "include/hw_controller.h"

ledc_timer_config_t ledc_timer;
ledc_channel_config_t ledc_channels[9] = {0}; // last is always motor

uint8_t blinker_mode = 0;
uint32_t next_duty_cycle = 0;

static void set_blinker_left(uint32_t duty_cycle) {
    ledc_set_duty(LEDC_MODE, LEDC_LEFT_BLINKER_CHANNEL, duty_cycle);
    ledc_update_duty(LEDC_MODE, LEDC_LEFT_BLINKER_CHANNEL);
}

static void set_blinker_right(uint32_t duty_cycle) {
    ledc_set_duty(LEDC_MODE, LEDC_RIGHT_BLINKER_CHANNEL, duty_cycle);
    ledc_update_duty(LEDC_MODE, LEDC_RIGHT_BLINKER_CHANNEL);
}

static void blinker_task() {
    #ifdef CONFIG_LIGHT_CONTROL
    while (1) {
        if (blinker_mode == 1) {
            set_blinker_left(next_duty_cycle);
            set_blinker_right(0);
        } else if (blinker_mode == 2) {
            set_blinker_left(0);
            set_blinker_right(next_duty_cycle);
        } else if (blinker_mode == 3) {
            set_blinker_left(next_duty_cycle);
            set_blinker_right(next_duty_cycle);
        } else {
            set_blinker_left(0);
            set_blinker_right(0);
        }
        if (next_duty_cycle == 0) {
            next_duty_cycle = LEDC_DUTY_50;
        } else {
            next_duty_cycle = 0;
        }
        vTaskDelay(CONFIG_BLINKER_INTERVAL / portTICK_PERIOD_MS);
    }
    #else
    return;
    #endif
}

void init_hw_controller() {
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .timer_num        = LEDC_TIMER,
        .duty_resolution  = LEDC_DUTY_RES,
        .freq_hz          = LEDC_FREQUENCY,  // Set output frequency at 5 kHz
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));
    
    #ifdef CONFIG_LIGHT_CONTROL
    #ifdef CONFIG_HEADLIGHT_INDIVIDUAL
        ledc_channels[HEADLIGHT_LEFT_INDEX] = (ledc_channel_config_t){
            .speed_mode     = LEDC_MODE,
            .channel        = LEDC_HEADLIGHT_CHANNEL,
            .timer_sel      = LEDC_TIMER,
            .intr_type      = LEDC_INTR_DISABLE,
            .gpio_num       = CONFIG_HEADLIGHT_LEFT_PIN,
            .duty           = 0, // Set duty to 0%
            .hpoint         = 0
        };
        ESP_ERROR_CHECK(ledc_channel_config(&ledc_channels[HEADLIGHT_LEFT_INDEX]));
        ledc_channels[HEADLIGHT_RIGHT_INDEX] = (ledc_channel_config_t){
            .speed_mode     = LEDC_MODE,
            .channel        = LEDC_HEADLIGHT_CHANNEL,
            .timer_sel      = LEDC_TIMER,
            .intr_type      = LEDC_INTR_DISABLE,
            .gpio_num       = CONFIG_HEADLIGHT_RIGHT_PIN,
            .duty           = 0, // Set duty to 0%
            .hpoint         = 0
        };
        ESP_ERROR_CHECK(ledc_channel_config(&ledc_channels[HEADLIGHT_RIGHT_INDEX]));
    #else 
        ledc_channels[HEADLIGHT_INDEX] = (ledc_channel_config_t){
            .speed_mode     = LEDC_MODE,
            .channel        = LEDC_HEADLIGHT_CHANNEL,
            .timer_sel      = LEDC_TIMER,
            .intr_type      = LEDC_INTR_DISABLE,
            .gpio_num       = CONFIG_HEADLIGHT_PIN,
            .duty           = 0, // Set duty to 0%
            .hpoint         = 0
        };
        ESP_ERROR_CHECK(ledc_channel_config(&ledc_channels[HEADLIGHT_INDEX]));
    #endif

    #ifdef CONFIG_TAILLIGHT_INDIVIDUAL
        ledc_channels[TAILLIGHT_LEFT_INDEX] = (ledc_channel_config_t){
            .speed_mode     = LEDC_MODE,
            .channel        = LEDC_TAILLIGHT_CHANNEL,
            .timer_sel      = LEDC_TIMER,
            .intr_type      = LEDC_INTR_DISABLE,
            .gpio_num       = CONFIG_TAILLIGHT_LEFT_PIN,
            .duty           = 0, // Set duty to 0%
            .hpoint         = 0
        };
        ESP_ERROR_CHECK(ledc_channel_config(&ledc_channels[TAILLIGHT_LEFT_INDEX]));
        ledc_channels[TAILLIGHT_RIGHT_INDEX] = (ledc_channel_config_t){
            .speed_mode     = LEDC_MODE,
            .channel        = LEDC_TAILLIGHT_CHANNEL,
            .timer_sel      = LEDC_TIMER,
            .intr_type      = LEDC_INTR_DISABLE,
            .gpio_num       = CONFIG_TAILLIGHT_RIGHT_PIN,
            .duty           = 0, // Set duty to 0%
            .hpoint         = 0
        };
        ESP_ERROR_CHECK(ledc_channel_config(&ledc_channels[TAILLIGHT_RIGHT_INDEX]));
    #else
        ledc_channels[TAILLIGHT_INDEX] = (ledc_channel_config_t){
            .speed_mode     = LEDC_MODE,
            .channel        = LEDC_TAILLIGHT_CHANNEL,
            .timer_sel      = LEDC_TIMER,
            .intr_type      = LEDC_INTR_DISABLE,
            .gpio_num       = CONFIG_TAILLIGHT_PIN,
            .duty           = 0, // Set duty to 0%
            .hpoint         = 0
        };
        ESP_ERROR_CHECK(ledc_channel_config(&ledc_channels[TAILLIGHT_INDEX]));
    #endif

    #ifdef CONFIG_BLINKER_INDIVIDUAL
        ledc_channels[BLINKER_FRONT_LEFT_INDEX] = (ledc_channel_config_t){
            .speed_mode     = LEDC_MODE,
            .channel        = LEDC_LEFT_BLINKER_CHANNEL,
            .timer_sel      = LEDC_TIMER,
            .intr_type      = LEDC_INTR_DISABLE,
            .gpio_num       = CONFIG_BLINKER_FRONT_LEFT_PIN,
            .duty           = 0, // Set duty to 0%
            .hpoint         = 0
        };
        ESP_ERROR_CHECK(ledc_channel_config(&ledc_channels[BLINKER_FRONT_LEFT_INDEX]));
        ledc_channels[BLINKER_REAR_LEFT_INDEX] = (ledc_channel_config_t){
            .speed_mode     = LEDC_MODE,
            .channel        = LEDC_LEFT_BLINKER_CHANNEL,
            .timer_sel      = LEDC_TIMER,
            .intr_type      = LEDC_INTR_DISABLE,
            .gpio_num       = CONFIG_BLINKER_REAR_LEFT_PIN,
            .duty           = 0, // Set duty to 0%
            .hpoint         = 0
        };
        ESP_ERROR_CHECK(ledc_channel_config(&ledc_channels[BLINKER_REAR_LEFT_INDEX]));
        ledc_channels[BLINKER_FRONT_RIGHT_INDEX] = (ledc_channel_config_t){
            .speed_mode     = LEDC_MODE,
            .channel        = LEDC_RIGHT_BLINKER_CHANNEL,
            .timer_sel      = LEDC_TIMER,
            .intr_type      = LEDC_INTR_DISABLE,
            .gpio_num       = CONFIG_BLINKER_FRONT_RIGHT_PIN,
            .duty           = 0, // Set duty to 0%
            .hpoint         = 0
        };
        ESP_ERROR_CHECK(ledc_channel_config(&ledc_channels[BLINKER_FRONT_RIGHT_INDEX]));
        ledc_channels[BLINKER_REAR_RIGHT_INDEX] = (ledc_channel_config_t){
            .speed_mode     = LEDC_MODE,
            .channel        = LEDC_RIGHT_BLINKER_CHANNEL,
            .timer_sel      = LEDC_TIMER,
            .intr_type      = LEDC_INTR_DISABLE,
            .gpio_num       = CONFIG_BLINKER_REAR_RIGHT_PIN,
            .duty           = 0, // Set duty to 0%
            .hpoint         = 0
        };
        ESP_ERROR_CHECK(ledc_channel_config(&ledc_channels[BLINKER_REAR_RIGHT_INDEX]));
    #else
        ledc_channels[BLINKER_LEFT_INDEX] = (ledc_channel_config_t){
            .speed_mode     = LEDC_MODE,
            .channel        = LEDC_LEFT_BLINKER_CHANNEL,
            .timer_sel      = LEDC_TIMER,
            .intr_type      = LEDC_INTR_DISABLE,
            .gpio_num       = CONFIG_BLINKER_LEFT_PIN,
            .duty           = 0, // Set duty to 0%
            .hpoint         = 0
        };
        ESP_ERROR_CHECK(ledc_channel_config(&ledc_channels[BLINKER_LEFT_INDEX]));
        ledc_channels[BLINKER_RIGHT_INDEX] = (ledc_channel_config_t){
            .speed_mode     = LEDC_MODE,
            .channel        = LEDC_RIGHT_BLINKER_CHANNEL,
            .timer_sel      = LEDC_TIMER,
            .intr_type      = LEDC_INTR_DISABLE,
            .gpio_num       = CONFIG_BLINKER_RIGHT_PIN,
            .duty           = 0, // Set duty to 0%
            .hpoint         = 0
        };
        ESP_ERROR_CHECK(ledc_channel_config(&ledc_channels[BLINKER_RIGHT_INDEX]));
    #endif

    xTaskCreate(&blinker_task, "blinker_task", 2048, NULL, 5, NULL);
    #endif

    #ifdef CONFIG_MOTOR_PIN
    ledc_channel_config_t ledc_motor_channel = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_MOTOR_CHANNEL,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = CONFIG_MOTOR_PIN,
        .duty           = 0, // Set duty to 0%
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_motor_channel));
    #endif
}

void set_headlight_mode(uint8_t mode) {
    #ifdef CONFIG_LIGHT_CONTROL
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
    #endif
}

void set_taillight_mode(uint8_t mode) {
    #ifdef CONFIG_LIGHT_CONTROL
    if (mode == 1) {
        ledc_set_duty(LEDC_MODE, LEDC_TAILLIGHT_CHANNEL, LEDC_DUTY_25);
        ledc_update_duty(LEDC_MODE, LEDC_TAILLIGHT_CHANNEL);
    } else if (mode == 2) {
        ledc_set_duty(LEDC_MODE, LEDC_TAILLIGHT_CHANNEL, LEDC_DUTY_50);
        ledc_update_duty(LEDC_MODE, LEDC_TAILLIGHT_CHANNEL);
    } else {
        ledc_set_duty(LEDC_MODE, LEDC_TAILLIGHT_CHANNEL, 0);
        ledc_update_duty(LEDC_MODE, LEDC_TAILLIGHT_CHANNEL);
    }
    #endif
}

void set_blinker_mode(uint8_t mode) {
    #ifdef CONFIG_LIGHT_CONTROL
    blinker_mode = mode;
    next_duty_cycle = LEDC_DUTY_50;
    #endif
}

void set_motor_duty(uint8_t duty_cycle) {
    #ifdef CONFIG_MOTOR_PIN
    uint32_t duty = duty_cycle;
    if (duty_cycle > CONFIG_MOTOR_MAX_DUTY_CYCLE) {
        duty = CONFIG_MOTOR_MAX_DUTY_CYCLE;
    }
    uint32_t ledc_duty = pow(2,LEDC_DUTY_RES) -1;
    ledc_duty = ledc_duty * duty;
    ledc_duty = ledc_duty / 100;
    ledc_set_duty(LEDC_MODE, LEDC_MOTOR_CHANNEL, ledc_duty);
    ledc_update_duty(LEDC_MODE, LEDC_MOTOR_CHANNEL);
    #endif
}