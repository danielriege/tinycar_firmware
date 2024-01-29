#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>
#include "esp_err.h"
#include "esp_log.h"

#include <string.h>
#include <sys/param.h>
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_timer.h"

#define MAX_FPS 30

typedef struct {
    // delay between frames
    uint16_t tranmission_interval; // milliseconds 
} congestion_control_t;

void update_congestion_control(congestion_control_t* congestion_control_parameters);

congestion_control_t congestion_control_init();