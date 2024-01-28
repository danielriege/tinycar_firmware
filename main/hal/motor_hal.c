#include "motor_hal.h"

static const char *TAG = "motor";

static ledc_timer_config_t motor_timer;
static ledc_channel_config_t motor_channel_forward_config;
static ledc_channel_config_t motor_channel_reverse_config;

void init_motor(void) {
    motor_timer = (ledc_timer_config_t) {
        .speed_mode       = MOTOR_MODE,
        .timer_num        = MOTOR_TIMER,
        .duty_resolution  = MOTOR_DUTY_RES,
        .freq_hz          = MOTOR_PWM_FREQ,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&motor_timer));

    motor_channel_forward_config = (ledc_channel_config_t){
        .speed_mode     = MOTOR_MODE,
        .channel        = MOTOR_FORWARD_CHANNEL,
        .timer_sel      = MOTOR_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = PIN_MOTOR_FIN,
        .duty           = 0, // Set duty to 0%
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&motor_channel_forward_config));

    motor_channel_reverse_config = (ledc_channel_config_t){
        .speed_mode     = MOTOR_MODE,
        .channel        = MOTOR_REVERSE_CHANNEL,
        .timer_sel      = MOTOR_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = PIN_MOTOR_RIN,
        .duty           = 0, // Set duty to 0%
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&motor_channel_reverse_config));
}

void motor_set_duty_cycle(int8_t duty_cycle) {
    if (duty_cycle > MOTOR_MAX_DUTY) {
        duty_cycle = MOTOR_MAX_DUTY;
    }
    if (duty_cycle < -MOTOR_MAX_DUTY) {
        duty_cycle = -MOTOR_MAX_DUTY;
    }
    if (MOTOR_INVERT_DIR) {
        duty_cycle = duty_cycle * -1;
    }
    uint16_t ledc_duty_cycle = MOTOR_PWM_RES_HIGH * (fabs(duty_cycle) / 100.0);
    if (duty_cycle > 0) {
        ledc_set_duty(MOTOR_MODE, MOTOR_FORWARD_CHANNEL, ledc_duty_cycle);
        ledc_update_duty(MOTOR_MODE, MOTOR_FORWARD_CHANNEL);
        ledc_set_duty(MOTOR_MODE, MOTOR_REVERSE_CHANNEL, 0);
        ledc_update_duty(MOTOR_MODE, MOTOR_REVERSE_CHANNEL);
    } else if (duty_cycle < 0) {
        ledc_set_duty(MOTOR_MODE, MOTOR_FORWARD_CHANNEL, 0);
        ledc_update_duty(MOTOR_MODE, MOTOR_FORWARD_CHANNEL);
        ledc_set_duty(MOTOR_MODE, MOTOR_REVERSE_CHANNEL, ledc_duty_cycle);
        ledc_update_duty(MOTOR_MODE, MOTOR_REVERSE_CHANNEL);
    } else {
        ledc_set_duty(MOTOR_MODE, MOTOR_FORWARD_CHANNEL, 0);
        ledc_update_duty(MOTOR_MODE, MOTOR_FORWARD_CHANNEL);
        ledc_set_duty(MOTOR_MODE, MOTOR_REVERSE_CHANNEL, 0);
        ledc_update_duty(MOTOR_MODE, MOTOR_REVERSE_CHANNEL);
    }
}