#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>
#include "esp_err.h"
#include "esp_log.h"

#include "esp_camera.h"

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

#include "protocol.h"

/// @brief Returns a pointer to the buffer used for sending data. Changing data on this buffer directly prevents a memcpy.
/// @return pointer to buffer, can be casted to outgoing events from protocol.h
uint8_t* cc_get_buffer();

/// @brief Sets a new current address to which data will be sent on cc_send_data
///
/// @param dest_ip destination ip
/// @param dest_port destination port
void cc_set_address(in_addr_t dest_ip, uint16_t dest_port);

/// @brief Sends data to the current address. It is recommended to use cc_get_buffer and use that pointer as data to prevent a memcpy.
///
/// @param message_type message type (use from protocol.h)
/// @param data data to send
/// @param data_len length of data
void cc_send_data(uint8_t message_type, uint8_t* data, size_t data_len);
