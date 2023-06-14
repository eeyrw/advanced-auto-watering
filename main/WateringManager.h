#ifndef WATERING_MANAGER_H
#define WATERING_MANAGER_H

#include "esp_system.h"
#include <type_traits>
#include "WateringManager.h"
#include <stdio.h>
#include "nvs_flash.h"
#include "nvs.h"
#include "nvs_handle.hpp"
#include "esp_err.h"
#include "esp_system.h"
#include "esp_log.h"

class WateringManager
{
private:
    /* data */
    int wateringIntervalSec;
    int wateringDurationSec;
    int lastWateringTimestamp;

    void InitConfigFromNVS(void);

public:
    WateringManager(/* args */);
    ~WateringManager();
};
static const char *TAG = "WATERING_MANAGER";
const int wateringIntervalSec_default = 12 * 60 * 60;
const int wateringDurationSec_default = 60;

template <typename T>
esp_err_t NVSReadWithDefault(std::unique_ptr<nvs::NVSHandle> &handle, const char *key, T &value, const T &defaultValue)
{
    esp_err_t err;
    // Read
    ESP_LOGI(TAG, "Reading %s from NVS ... ", key);
    err = handle->get_item(key, value);
    switch (err)
    {
    case ESP_OK:
        ESP_LOGI(TAG, "%s = %d", key, value);
        break;
    case ESP_ERR_NVS_NOT_FOUND:
        ESP_LOGI(TAG, "The value is not initialized yet!");
        handle->set_item(key, defaultValue);
        break;
    default:
        ESP_LOGI(TAG, "Error (%s) reading!\n", esp_err_to_name(err));
    }
    return err;
}

#endif