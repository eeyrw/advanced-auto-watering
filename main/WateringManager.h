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
    int waterIntvlSec;
    int waterDurSec;
    int lastWateringTimestamp;

    void InitConfigFromNVS(void);

public:
    WateringManager(/* args */);
    ~WateringManager();
};
static const char *TAG = "WATERING_MANAGER";
const int waterIntvlSec_default = 12 * 60 * 60;
const int waterDurSec_default = 60;

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
        ESP_LOGW(TAG, "The value is not initialized yet!");
        err = handle->set_item(key, defaultValue);
        if(err!=ESP_OK)
        {
            ESP_LOGE(TAG, "Error (%s) set_item!\n", esp_err_to_name(err));
        }
        break;
    default:
        ESP_LOGE(TAG, "Error (%s) reading!\n", esp_err_to_name(err));
    }
    return err;
}

#endif