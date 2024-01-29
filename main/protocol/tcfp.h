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
    uint32_t timestamp;
    uint16_t frame_num;
    uint32_t seq_num: 6;
    uint32_t rtt_start: 1;
    uint32_t marker: 1;
    uint32_t fragment_offset: 24;
    uint8_t fragment_count;
    uint8_t width; // mutiple of 8
    uint8_t height; // mutiple of 8
} tcfp_header_t;

void tcfp_set_destination(in_addr_t* dest_ip);

int tcfp_send_frame(uint8_t *frame, size_t size, uint16_t width, uint16_t height, uint8_t rtt_start);

/// @brief Gets the number of packets needed the last time for a frame. Is 0 if no frame has been sent.
uint32_t tcfp_get_last_packet_count();
