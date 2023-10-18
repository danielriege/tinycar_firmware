#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>
#include "driver/ledc.h"
#include "esp_err.h"
#include "esp_log.h"
#include <math.h>

#define SERVO_TIMER              LEDC_TIMER_1
#define SERVO_MODE               LEDC_LOW_SPEED_MODE
#define SERVO_DUTY_RES           LEDC_TIMER_13_BIT // Set duty resolution to 13 bits
#define SERVO_FREQUENCY          (50) // Frequency in Hertz. 

#define SERVO_CHANNEL           LEDC_CHANNEL_6
#define SERVO_GPIO              GPIO_NUM_3

#define SERVO_DUTY_CYCLE_MIN    (204) // 0.5 ms (8191 * 0.025)
#define SERVO_DUTY_CYCLE_MAX    (1023) // 2.5 ms (8191 * 0.125)

void init_servo(void);
void servo_set_duty_cycle(uint16_t duty_cycle);

/// @brief Set the angle of the servo
/// @param angle angle in *100 degrees (180 deg = 18000)
void servo_set_angle(uint16_t angle);