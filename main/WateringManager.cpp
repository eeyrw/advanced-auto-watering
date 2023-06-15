#include "WateringManager.h"
#include <stdio.h>
#include "nvs_flash.h"
#include "nvs.h"
#include "nvs_handle.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include <any>
#include <vector>
#include <string>
#include <map>
#include "GlobalConfig.h"

static const char *TAG = "WATERING_MANAGER";

void WateringManager::InitConfigFromNVS(void)
{
    esp_err_t err;
    int t = 0;

    // err = ConfigWrite("watering_config", "waterDurSec", t);
    // ESP_LOGI(TAG,"Write waterDurSec=%d",t);
    //err = ConfigRead("watering_config", "waterDurSec", t);
    //ESP_LOGI(TAG,"Read waterDurSec=%d",t);
    std::string ssw("Hahaha~!");
    std::string ss;
    // ConfigWrite("watering_config","waterMode",ssw);
    // ESP_LOGI(TAG,"Write waterMode=%s",ssw.c_str());
    ConfigRead("watering_config","waterNm",ss);
    ESP_LOGI(TAG,"Read waterNm=%s",ss.c_str());
}

WateringManager::WateringManager(/* args */)
{
    InitConfigFromNVS();
}

WateringManager::~WateringManager()
{
}
