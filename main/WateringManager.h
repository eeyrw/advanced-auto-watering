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

#endif