#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>
#include "esp_err.h"
#include "esp_log.h"

#include "driver/adc.h"
#include "driver/gpio.h"
#include "esp_adc_cal.h"

#include "../protocol/tccp.h"
#include "pinout.h"

void battery_status_init();
int battery_status_read();