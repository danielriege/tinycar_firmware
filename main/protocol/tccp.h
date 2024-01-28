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
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_netif.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#define TCCP_PORT CONFIG_UDP_PORT

#define TCCP_TYPE_CONTROL 0x00
#define TCCP_TYPE_TELEMETRY 0x01

typedef struct {
    uint8_t type: 1; // 0 = control, 1 = telemetry
    uint8_t padding: 7;
} tccp_header_t;

typedef struct {
    tccp_header_t header;
    uint16_t battery_voltage;
    uint8_t current_fps; 
    uint16_t min_frame_latency;
    int8_t wifi_rssi;
} tccp_telemetry_t;

typedef struct {
    tccp_header_t header;
    uint8_t headlight:2; // 0 = off, 1 = on, 2 = full beam
    uint8_t taillight:2; // 0 = off, 1 = on, 2 = brake
    uint8_t blinker:2; // 0 = off, 1 = left, 2 = right, 3 = hazard
    uint8_t padding:2;
    uint16_t servo_angle; // 0-18000
    int8_t motor_duty_cycle; // -255-255
} tccp_control_t;

ESP_EVENT_DECLARE_BASE(CONTROL_EVENT);

enum {
    TCCP_CONTROL_EVENT
};

void init_tccp(void);
void start_tccp(void);
void register_tccp_event_handler(void (*handler)(void*, esp_event_base_t, int32_t, void*));
void send_telemetry(uint16_t battery_voltage, uint8_t current_fps, uint16_t min_frame_latency, int8_t wifi_rssi);
in_addr_t* get_address_ref();