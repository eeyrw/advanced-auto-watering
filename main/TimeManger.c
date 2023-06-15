#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "freertos/event_groups.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include <time.h>
#include <sys/time.h>
#include "esp_sntp.h"
#include "esp_netif.h"
#include "TimeManger.h"
#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

static const char *TAG = "TIME_MANAGER";
RTC_DATA_ATTR time_t lastSyncTime;
void time_init_task(void *arg)
{
    time_t now;
    struct tm timeinfo;
    char strftime_buf[64];
    // Set timezone to China Standard Time
    setenv("TZ", "CST-8", 1);
    tzset();
    time(&now);
    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGI(TAG, "The date/time in Shanghai before syncing to NTP is: %s", strftime_buf);
    double deltaSec = difftime(now, lastSyncTime);
    if (deltaSec > 60 * 30 || timeinfo.tm_year < (2023 - 1900))
    {
        localtime_r(&lastSyncTime, &timeinfo);
        strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
        ESP_LOGI(TAG, "Sync from NTP is: %s", strftime_buf);
        ESP_LOGW(TAG, "Time is not set yet. Connecting to WiFi and getting time over NTP.");
        obtain_time();
        time(&now);
        lastSyncTime = now;
        localtime_r(&now, &timeinfo);
        strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
        ESP_LOGI(TAG, "The current date/time in Shanghai after syncing to NTP is: %s", strftime_buf);
    }
    vTaskDelete(NULL);
}

void obtain_time(void)
{
    initialize_sntp();

    // wait for time to be set
    time_t now = 0;
    struct tm timeinfo = {0};
    int retry = 0;
    const int retry_count = 15;
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET)
    {
        if (++retry >= retry_count)
        {
            ESP_LOGE(TAG, "Fail to sync time by NTP within %d tries.", retry_count);
            break;
        }
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
    time(&now);
    localtime_r(&now, &timeinfo);
}

void initialize_sntp(void)
{
    ESP_LOGI(TAG, "Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);

    // otherwise, use DNS address from a pool
    sntp_setservername(0, "cn.pool.ntp.org");
    sntp_setservername(1, "pool.ntp.org"); // set the secondary NTP server (will be used only if SNTP_MAX_SERVERS > 1)
    sntp_init();

    ESP_LOGI(TAG, "List of configured NTP servers:");

    for (uint8_t i = 0; i < SNTP_MAX_SERVERS; ++i)
    {
        if (sntp_getservername(i))
        {
            ESP_LOGI(TAG, "server %d: %s", i, sntp_getservername(i));
        }
        else
        {
            // we have either IPv4 or IPv6 address, let's print it
            char buff[INET6_ADDRSTRLEN];
            ip_addr_t const *ip = sntp_getserver(i);
            if (ipaddr_ntoa_r(ip, buff, INET6_ADDRSTRLEN) != NULL)
                ESP_LOGI(TAG, "server %d: %s", i, buff);
        }
    }
}