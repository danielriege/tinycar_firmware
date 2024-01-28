#include "storage.h"

static const char *TAG = "storage";

esp_err_t storage_init() {
    ESP_LOGI(TAG, "Initializing NVS");
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES) {
        ESP_LOGI(TAG, "NVS partition is full, erasing...");
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
    return err;
}

static void storage_read_string(char *key, char **value) {
    nvs_handle_t my_handle;
    esp_err_t err;
    err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGI(TAG, "NVS entry not found");
    } else {
        size_t required_size;
        err = nvs_get_str(my_handle, key, NULL, &required_size);
        if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) {
            ESP_LOGI(TAG, "NVS entry not found");
        } else {
            *value = malloc(required_size);
            err = nvs_get_str(my_handle, key, *value, &required_size);
            if (err != ESP_OK) {
                ESP_LOGE(TAG, "Error (%s) reading!", esp_err_to_name(err));
            }
        }
    }
    nvs_close(my_handle);
}

static void storage_write_string(char *key, char *value) {
    nvs_handle_t my_handle;
    esp_err_t err;
    err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
    } else {
        err = nvs_set_str(my_handle, key, value);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Error (%s) writing!", esp_err_to_name(err));
        }
        err = nvs_commit(my_handle);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Error (%s) committing!", esp_err_to_name(err));
        }
    }
    nvs_close(my_handle);
}

void storage_read_wifi_credentials(char **ssid, char **password) {
    storage_read_string(NVS_KEY_SSID, ssid);
    storage_read_string(NVS_KEY_PASS, password);
}

void storage_write_wifi_credentials(char *ssid, char *password) {
    storage_write_string(NVS_KEY_SSID, ssid);
    storage_write_string(NVS_KEY_PASS, password);
}