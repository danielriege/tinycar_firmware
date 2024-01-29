#include "battery_status_hal.h"

#define R8 100
#define R9 10

static const char *TAG = "battery status";

static esp_adc_cal_characteristics_t adc1_chars;

static int adc_voltage_to_battery_voltage(int adc_voltage) {
    return adc_voltage * (R8 + R9) / R9;
}

void battery_status_init() {
    // configure ADC EN
    gpio_set_direction(PIN_BAT_ADC_EN, GPIO_MODE_OUTPUT);
    // configure ADC
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_0, ADC_WIDTH_BIT_DEFAULT, 0, &adc1_chars);

    ESP_ERROR_CHECK(adc1_config_width(ADC_WIDTH_BIT_12));
    ESP_ERROR_CHECK(adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_0));

    // since capacitor takes several 100 ms to charge, reading this value immediately after enabling ADC will give wrong value
    gpio_set_level(PIN_BAT_ADC_EN, 1);
}

int battery_status_read() {
    #ifdef CONFIG_BATTERY_MEASUREMENT
    //gpio_set_level(PIN_BAT_ADC_EN, 1);
    //int adc_voltage = adc_to_voltage(adc1_get_raw(ADC1_CHANNEL_0)); // in mV
    int adc_voltage = esp_adc_cal_raw_to_voltage(adc1_get_raw(ADC1_CHANNEL_0), &adc1_chars);
    // voltage is 0.45 V for 5V input after voltage divider
    int voltage = adc_voltage_to_battery_voltage(adc_voltage);
    //gpio_set_level(PIN_BAT_ADC_EN, 0);
    return voltage;
    #else
    return 0;
    #endif
}