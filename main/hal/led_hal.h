#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>
#include "driver/ledc.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_check.h"
#include "soc/io_mux_reg.h"
#include <math.h>

#include "pinout.h"

#define LEDC_TIMER              LEDC_TIMER_2
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_DUTY_RES           LEDC_TIMER_13_BIT // Set duty resolution to 13 bits
#define LEDC_FREQUENCY          (5000) // Frequency in Hertz. Set frequency at 5 kHz

#define LEDC_DUTY_25            (2047) // Set duty to 25%. ((2 ** 13) - 1) * 25% = 2047
#define LEDC_DUTY_50            (4095) // Set duty to 50%. ((2 ** 13) - 1) * 50% = 4095
#define LEDC_DUTY_75            (6143) // Set duty to 75%. ((2 ** 13) - 1) * 75% = 6143‚
#define LEDC_DUTY_100           (8191) // Set duty to 100%. ((2 ** 13) - 1) * 100% = 8191

#define LEDC_HEADLIGHT_CHANNEL  LEDC_CHANNEL_1
#define LEDC_TAILLIGHT_CHANNEL  LEDC_CHANNEL_2

#ifdef CONFIG_BLINKER_INTERVAL
    #define BLINKER_INTERVAL  (CONFIG_BLINKER_INTERVAL / portTICK_PERIOD_MS)
#else
    #define BLINKER_INTERVAL  (500 / portTICK_PERIOD_MS)
#endif

void init_led_controller(void);
void led_set_headlight_mode(uint8_t mode);
void led_set_taillight_mode(uint8_t mode);
void led_set_blinker_mode(uint8_t mode);

