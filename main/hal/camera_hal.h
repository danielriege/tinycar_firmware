#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>
#include "esp_err.h"
#include "esp_log.h"

#include "esp_camera.h"
#include "pinout.h"


esp_err_t camera_init();
void camera_set_config(uint8_t resolution, uint8_t jpeg_quality);

camera_fb_t* camera_get_frame_buffer();
void camera_return_frame_buffer(camera_fb_t* fb);
