#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>
#include "esp_err.h"
#include "esp_log.h"

#include "driver/adc.h"
#include "esp_adc_cal.h"

#include "control_client.h"

void battery_status_init();
int battery_status_read();
void send_battery_status(int);