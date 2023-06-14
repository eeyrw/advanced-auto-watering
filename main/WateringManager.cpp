#include "WateringManager.h"
#include <stdio.h>
#include "nvs_flash.h"
#include "nvs.h"
#include "nvs_handle.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"

void WateringManager::InitConfigFromNVS(void)
{
    esp_err_t err;
    // Handle will automatically close when going out of scope or when it's reset.
    std::unique_ptr<nvs::NVSHandle> handle = nvs::open_nvs_handle("watering_config", NVS_READWRITE, &err);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    }
    else
    {
        err = NVSReadWithDefault(handle, "waterIntvlSec", waterIntvlSec, waterIntvlSec_default);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Error (%s) NVSReadWithDefault!\n", esp_err_to_name(err));
        }
        err = NVSReadWithDefault(handle, "waterDurSec", waterDurSec, waterDurSec_default);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Error (%s) NVSReadWithDefault!\n", esp_err_to_name(err));
        }
        // Commit written value.
        // After setting any values, nvs_commit() must be called to ensure changes are written
        // to flash storage. Implementations may write to storage at other times,
        // but this is not guaranteed.
        ESP_LOGI(TAG, "Committing updates in NVS ... ");
        err = handle->commit();
        if (err != ESP_OK)
        {
            ESP_LOGI(TAG, "Error (%s) commit!\n", esp_err_to_name(err));
        }
    }
}

WateringManager::WateringManager(/* args */)
{
    InitConfigFromNVS();
}

WateringManager::~WateringManager()
{
}
