#include "include/battery_status.h"

#define R2 100
#define R3 10
#define VOLTAGE_OUT(Vin) (((Vin) * R3) / (R2 + R3))

#define GPIO_BAT_ADC 1
#define GPIO_BAT_ADC_EN 14

static esp_adc_cal_characteristics_t adc1_chars;

void battery_status_init() {
    // configure ADC
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_0, ADC_WIDTH_BIT_DEFAULT, 0, &adc1_chars);

    ESP_ERROR_CHECK(adc1_config_width(ADC_WIDTH_BIT_12));
    ESP_ERROR_CHECK(adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_0));
}

int battery_status_read() {
    voltage = esp_adc_cal_raw_to_voltage(adc1_get_raw(ADC1_CHANNEL_0), &adc1_chars);
    return voltage; 
}

void send_battery_status(int) {
    uint8_t battery_status[3];
    battery_status[0] = 0x02;
    battery_status[1] = voltage >> 8;
    battery_status[2] = voltage & 0xFF;
    send_data(battery_status, 3);
}