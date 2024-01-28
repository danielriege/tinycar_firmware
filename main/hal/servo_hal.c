#include "servo_hal.h"

static const char *TAG = "servo";

static ledc_timer_config_t servo_timer;
static ledc_channel_config_t servo_channel_config; // last is always motor

void init_servo(void) {
    servo_timer = (ledc_timer_config_t) {
        .speed_mode       = SERVO_MODE,
        .timer_num        = SERVO_TIMER,
        .duty_resolution  = SERVO_DUTY_RES,
        .freq_hz          = SERVO_FREQUENCY,  // Set output frequency at 5 Hz (20ms duty cycle)
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&servo_timer));

    servo_channel_config = (ledc_channel_config_t){
        .speed_mode     = SERVO_MODE,
        .channel        = SERVO_CHANNEL,
        .timer_sel      = SERVO_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = SERVO_GPIO,
        .duty           = 0, // Set duty to 0%
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&servo_channel_config));
}

void servo_set_duty_cycle(uint16_t duty_cycle) {
    ledc_set_duty(SERVO_MODE, SERVO_CHANNEL, duty_cycle);
    ledc_update_duty(SERVO_MODE, SERVO_CHANNEL);
}

void servo_set_angle(uint16_t angle) {
    #ifdef CONFIG_SERVO_MAX_STEERING_ANGLE
    uint16_t max_angle = (CONFIG_SERVO_MAX_STEERING_ANGLE * 100) + 9000;
    uint16_t min_angle = 9000 - (CONFIG_SERVO_MAX_STEERING_ANGLE * 100);
    if (angle > max_angle) {
        angle = max_angle;
    }
    if (angle < min_angle) {
        angle = min_angle;
    }
    uint16_t duty_cycle = (uint16_t) (SERVO_DUTY_CYCLE_MIN + (SERVO_DUTY_CYCLE_MAX - SERVO_DUTY_CYCLE_MIN) * angle / 18000);
    servo_set_duty_cycle(duty_cycle);
    #endif
}