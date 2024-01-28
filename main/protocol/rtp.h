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
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_netif.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#define RTP_PORT 4998
#define DGRAM_SIZE 1024

typedef struct {
    uint8_t version:2;
    uint8_t padding:1;
    uint8_t extension:1;
    uint8_t csrc_count:4;
    uint8_t marker:1;
    uint8_t payload_type:7;
    uint16_t sequence_number;
    uint32_t timestamp;
    uint32_t ssrc;
} rtp_header_t;

typedef struct {
    uint32_t type_specific:8;
    uint32_t fragment_offset:24;
    uint8_t type;
    uint8_t q;
    uint8_t width; // multiple of 8
    uint8_t height; // multiple of 8
} jpeg_header_t;

void rtp_set_destination(in_addr_t* dest_ip);

int rtp_send_frame(uint8_t *frame, size_t size, uint16_t width, uint16_t height);

/// @brief Sets the delay between sending RTP packets in microseconds for a frame
/// @param delay in milliseconds
void rtp_set_packet_delay(uint8_t delay);

/// @brief Gets the number of packets needed the last time for a frame. Is 0 if no frame has been sent.
uint32_t rtp_get_last_packet_count();
