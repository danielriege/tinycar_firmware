#pragma once

#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_netif.h"

#define DGRAM_SIZE  1024

//
// INCOMING

#define PROTOCOL_IN_CTRL_LIGHT 0x01
#define PROTOCOL_IN_CTRL_MOTOR 0x02
#define PROTOCOL_IN_CAMERA_CAP 0x03
#define PROTOCOL_IN_CAMERA_CFG 0x04
#define PROTOCOL_IN_BATTERY_REQ 0x05
#define PROTOCOL_IN_SERVO_CTRL 0x06

// This can be used for all events requiring a response
typedef struct {
    in_addr_t dest_ip;
} response_address_event_t;

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
    response_address_event_t response_address;
} camera_capture_event_t;

typedef struct {
    uint8_t resolution;
    uint8_t jpeg_quality;
    response_address_event_t response_address;
} camera_config_event_t;

typedef struct {
    response_address_event_t response_address;
} battery_status_req_event_t;

typedef struct {
    uint16_t angle;
} servo_control_event_t;

//
// OUTGOING
//

#define PROTOCOL_OUT_FRAME_METADATA 0x01
#define PROTOCOL_OUT_FRAME_FRAGMENT 0x02
#define PROTOCOL_OUT_BATTERY_STATUS 0x03

// for frames
typedef struct {
    uint32_t tag;
    uint16_t height;
    uint16_t width;
    uint8_t pixel_format;
    uint32_t frame_size;
} frame_metadata_event_t;

typedef struct {
    uint32_t tag;
    uint32_t fragment_id;
    uint16_t fragment_size;
} frame_fragment_event_t;

// for battery status
typedef struct {
    uint16_t voltage;
} battery_status_event_t;


