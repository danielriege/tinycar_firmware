#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "driver/gpio.h"

#include "esp_log.h"
#include "nvs_flash.h"

#define NVS_NAMESPACE "tinycar"

#define NVS_KEY_SSID "ssid"
#define NVS_KEY_PASS "pass"

esp_err_t storage_init();

// Reading values
void storage_read_wifi_credentials(char **ssid, char **pass);
// Writing values
void storage_write_wifi_credentials(char *ssid, char *pass);