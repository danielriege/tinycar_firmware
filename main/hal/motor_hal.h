#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>
#include "driver/ledc.h"
#include "esp_err.h"
#include "esp_check.h"
#include <math.h>

#include "pinout.h"

#define MOTOR_PWM_FREQ CONFIG_MOTOR_PWM_FREQ
#define MOTOR_INVERT_DIR CONFIG_MOTOR_INVERT_DIR
#define MOTOR_MAX_DUTY CONFIG_MOTOR_MAX_DUTY_CYCLE

#define MOTOR_TIMER              LEDC_TIMER_2
#define MOTOR_MODE               LEDC_LOW_SPEED_MODE
#define MOTOR_DUTY_RES           LEDC_TIMER_10_BIT // Set duty resolution to 10 bits
#define MOTOR_FORWARD_CHANNEL    LEDC_CHANNEL_3
#define MOTOR_REVERSE_CHANNEL    LEDC_CHANNEL_4

#define MOTOR_PWM_RES_HIGH       (1023) // 100% duty cycle

void init_motor(void);
void motor_set_duty_cycle(int8_t duty_cycle);
