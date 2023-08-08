#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>
#include "driver/ledc.h"
#include "esp_err.h"
#include <math.h>

#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_DUTY_RES           LEDC_TIMER_13_BIT // Set duty resolution to 13 bits
#define LEDC_FREQUENCY          (5000) // Frequency in Hertz. Set frequency at 5 kHz

#define LEDC_DUTY_25            (2047) // Set duty to 25%. ((2 ** 13) - 1) * 25% = 2047
#define LEDC_DUTY_50            (4095) // Set duty to 50%. ((2 ** 13) - 1) * 50% = 4095
#define LEDC_DUTY_100           (8191) // Set duty to 100%. ((2 ** 13) - 1) * 100% = 8191

#define LEDC_HEADLIGHT_CHANNEL            LEDC_CHANNEL_0
#define LEDC_TAILLIGHT_CHANNEL            LEDC_CHANNEL_1
#define LEDC_LEFT_BLINKER_CHANNEL         LEDC_CHANNEL_2
#define LEDC_RIGHT_BLINKER_CHANNEL        LEDC_CHANNEL_3
#define LEDC_MOTOR_CHANNEL                LEDC_CHANNEL_4

#define HEADLIGHT_INDEX 0
#define HEADLIGHT_LEFT_INDEX 0
#define HEADLIGHT_RIGHT_INDEX 1
#define TAILLIGHT_INDEX 2
#define TAILLIGHT_LEFT_INDEX 2
#define TAILLIGHT_RIGHT_INDEX 3
#define BLINKER_LEFT_INDEX 4
#define BLINKER_FRONT_LEFT_INDEX 4
#define BLINKER_REAR_LEFT_INDEX 5
#define BLINKER_RIGHT_INDEX 6
#define BLINKER_FRONT_RIGHT_INDEX 6
#define BLINKER_REAR_RIGHT_INDEX 7
#define MOTOR_INDEX 8


void init_hw_controller(void);
void set_headlight_mode(uint8_t mode);
void set_taillight_mode(uint8_t mode);
void set_blinker_mode(uint8_t mode);
void set_motor_duty(uint8_t duty);

