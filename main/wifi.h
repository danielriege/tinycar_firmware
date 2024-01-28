#pragma once

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "driver/uart_vfs.h"

#include "lwip/err.h"
#include "lwip/sys.h"


/// @brief Initialize the wifi module in AP mode
/// @param ssid The SSID of the access point. 
/// @param password The password of the access point. 
/// @return 0 if successful, -1 if failed (could not connect to WiFi)
int wifi_init_sta(char* ssid, char* password);

int8_t wifi_read_rssi();