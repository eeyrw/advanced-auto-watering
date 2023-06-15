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
    int t;
    ConfigRead("watering_config", "waterDurSec", t);
    ConfigWrite("watering_config", "waterDurSec", t);
    std::string ss;
    ConfigRead("Watering_config","waterMode",ss);
}

WateringManager::WateringManager(/* args */)
{
    InitConfigFromNVS();
}

WateringManager::~WateringManager()
{
}
