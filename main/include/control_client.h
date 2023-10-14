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

void set_address(in_addr_t dest_ip, uint16_t dest_port);
void send_data(uint8_t* data, size_t data_len);
