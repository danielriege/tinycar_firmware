#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>
#include "esp_err.h"
#include "esp_log.h"

#include "esp_camera.h"

#include "control_client.h"
#include "protocol.h"


esp_err_t camera_init();
void camera_set_config(uint8_t resolution, uint8_t jpeg_quality);
esp_err_t camera_capture_and_send(uint32_t tag);