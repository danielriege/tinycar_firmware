#pragma once

#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
//#include "protocol_examples_common.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

ESP_EVENT_DECLARE_BASE(CONTROL_EVENT);

enum {
    CONTROL_LIGHT_EVENT, CONTROL_MOTOR_EVENT, CAMERA_CAPTURE_EVENT, CAMERA_CONFIG_EVENT, BATTERY_REQ_EVENT
};

typedef struct {
    uint8_t headlight;
    uint8_t taillight;
    uint8_t blinker;
} control_light_event_t;

typedef struct {
    uint8_t duty_cycle;
} control_motor_event_t;

typedef struct {
    uint32_t tag;
    in_addr_t dest_ip;
} camera_capture_event_t;\

typedef struct {
    uint8_t resolution;
    uint8_t jpeg_quality;
} camera_config_event_t;

void init_control_server(void);
void start_control_server(void);
void register_control_server_event_handler(void (*handler)(void*, esp_event_base_t, int32_t, void*));