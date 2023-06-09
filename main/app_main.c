/* MQTT (over TCP) Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"

#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_wpa2.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_smartconfig.h"
#include "driver/temp_sensor.h"
#include <time.h>
#include <sys/time.h>
#include "esp_sntp.h"
#include <sht3x.h>
#include "wifi_manager.h"
#include <button.h>

static const char *TAG = "AUTO_WATERING";
/* FreeRTOS event group to signal when we are connected & ready to make a request */
static EventGroupHandle_t s_wifi_event_group;
static esp_mqtt_client_handle_t mqtt_client;
/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
static const int CONNECTED_BIT = BIT0;
static const int ESPTOUCH_DONE_BIT = BIT1;

static void mqtt_app_start(void);

static void smartconfig_example_task(void *parm);
void read_temperature_humidity_task(void *arg);

static void obtain_time(void);
static void initialize_sntp(void);

void time_sync_notification_cb(struct timeval *tv)
{
    ESP_LOGI(TAG, "Notification of a time synchronization event");
}
static sht3x_t dev;
void read_temperature_humidity_task(void *pvParameters)
{
    float temperature;
    float humidity;

    sht3x_init_desc(&dev, CONFIG_EXAMPLE_SHT3X_ADDR, 0, CONFIG_EXAMPLE_I2C_MASTER_SDA, CONFIG_EXAMPLE_I2C_MASTER_SCL);
    sht3x_init(&dev);

    TickType_t last_wakeup = xTaskGetTickCount();

    while (1)
    {
        // perform one measurement and do something with the results
        if (ESP_OK == sht3x_measure(&dev, &temperature, &humidity))
            ESP_LOGI(TAG, "SHT3x Sensor: %.2f °C, %.2f %%", temperature, humidity);
        else
            ESP_LOGE(TAG, "Fail to read SHT3x");

        // wait until 5 seconds are over
        vTaskDelayUntil(&last_wakeup, pdMS_TO_TICKS(5000));
    }
}

static const char *states[] = {
    [BUTTON_PRESSED]      = "pressed",
    [BUTTON_RELEASED]     = "released",
    [BUTTON_CLICKED]      = "clicked",
    [BUTTON_PRESSED_LONG] = "pressed long",
};
static button_t btn1;
static void on_button(button_t *btn, button_state_t state)
{
    ESP_LOGI(TAG, "Boot button %s", states[state]);
}

void button_init_task(void *pvParameters)
{
    // First button connected between GPIO and GND
    // pressed logic level 0, no autorepeat
    btn1.gpio = 9;
    btn1.pressed_level = 0;
    btn1.internal_pull = true;
    btn1.autorepeat = false;
    btn1.callback = on_button;

    ESP_ERROR_CHECK(button_init(&btn1));
    vTaskDelete(NULL);
}

void blink_task(void *pvParameters)
{

    gpio_set_direction(GPIO_NUM_12, GPIO_MODE_OUTPUT);
    gpio_set_direction(GPIO_NUM_13, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NUM_13, 0);

    TickType_t last_wakeup = xTaskGetTickCount();

    while (1)
    {
        gpio_set_level(GPIO_NUM_12, 0);
        vTaskDelayUntil(&last_wakeup, pdMS_TO_TICKS(1000));
        // wait until 5 seconds are over
        gpio_set_level(GPIO_NUM_12, 1);
        vTaskDelayUntil(&last_wakeup, pdMS_TO_TICKS(50));
    }
}

void time_init_task(void *arg)
{
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    // Is time set? If not, tm_year will be (1970 - 1900).
    // if (timeinfo.tm_year < (2016 - 1900)) {
    ESP_LOGI(TAG, "Time is not set yet. Connecting to WiFi and getting time over NTP.");
    obtain_time();
    // update 'now' variable with current time
    time(&now);
    //}

    char strftime_buf[64];

    // Set timezone to China Standard Time
    setenv("TZ", "CST-8", 1);
    tzset();
    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGI(TAG, "The current date/time in Shanghai is: %s", strftime_buf);
    vTaskDelete(NULL);
}

static void obtain_time(void)
{

    initialize_sntp();

    // wait for time to be set
    time_t now = 0;
    struct tm timeinfo = {0};
    int retry = 0;
    const int retry_count = 15;
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count)
    {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
    time(&now);
    localtime_r(&now, &timeinfo);
}

static void initialize_sntp(void)
{
    ESP_LOGI(TAG, "Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);

    // otherwise, use DNS address from a pool
    sntp_setservername(0, "cn.pool.ntp.org");
    sntp_setservername(1, "pool.ntp.org"); // set the secondary NTP server (will be used only if SNTP_MAX_SERVERS > 1)
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);
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

void tempsensor_example(void *arg)
{
    char jsonBuf[1024];
    // Initialize touch pad peripheral, it will start a timer to run a filter
    ESP_LOGI(TAG, "Initializing Temperature sensor");
    float tsens_out;
    temp_sensor_config_t temp_sensor = TSENS_CONFIG_DEFAULT();
    temp_sensor_get_config(&temp_sensor);
    ESP_LOGI(TAG, "default dac %d, clk_div %d", temp_sensor.dac_offset, temp_sensor.clk_div);
    temp_sensor.dac_offset = TSENS_DAC_DEFAULT; // DEFAULT: range:-10℃ ~  80℃, error < 1℃.
    temp_sensor_set_config(temp_sensor);
    temp_sensor_start();
    ESP_LOGI(TAG, "Temperature sensor started");
    while (1)
    {
        vTaskDelay(5000 / portTICK_RATE_MS);
        temp_sensor_read_celsius(&tsens_out);
        ESP_LOGI(TAG, "Temperature out celsius %f°C", tsens_out);
        sprintf(jsonBuf, "{\"id\":123,\"dp\":%f}", tsens_out);
        int msg_id = esp_mqtt_client_publish(mqtt_client, "/topic/qos0", jsonBuf, 0, 0, 0);
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
    }
    vTaskDelete(NULL);
}

static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        // xTaskCreate(smartconfig_example_task, "smartconfig_example_task", 4096, NULL, 3, NULL);
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        ESP_LOGI(TAG, "WIFI_EVENT_STA_DISCONNECTED");
        esp_wifi_connect();
        xEventGroupClearBits(s_wifi_event_group, CONNECTED_BIT);
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        xEventGroupSetBits(s_wifi_event_group, CONNECTED_BIT);
    }
    else if (event_base == SC_EVENT && event_id == SC_EVENT_SCAN_DONE)
    {
        ESP_LOGI(TAG, "Scan done");
    }
    else if (event_base == SC_EVENT && event_id == SC_EVENT_FOUND_CHANNEL)
    {
        ESP_LOGI(TAG, "Found channel");
    }
    else if (event_base == SC_EVENT && event_id == SC_EVENT_GOT_SSID_PSWD)
    {
        ESP_LOGI(TAG, "Got SSID and password");

        smartconfig_event_got_ssid_pswd_t *evt = (smartconfig_event_got_ssid_pswd_t *)event_data;
        wifi_config_t wifi_config;
        uint8_t ssid[33] = {0};
        uint8_t password[65] = {0};
        uint8_t rvd_data[33] = {0};

        bzero(&wifi_config, sizeof(wifi_config_t));
        memcpy(wifi_config.sta.ssid, evt->ssid, sizeof(wifi_config.sta.ssid));
        memcpy(wifi_config.sta.password, evt->password, sizeof(wifi_config.sta.password));
        wifi_config.sta.bssid_set = evt->bssid_set;
        if (wifi_config.sta.bssid_set == true)
        {
            memcpy(wifi_config.sta.bssid, evt->bssid, sizeof(wifi_config.sta.bssid));
        }

        memcpy(ssid, evt->ssid, sizeof(evt->ssid));
        memcpy(password, evt->password, sizeof(evt->password));
        ESP_LOGI(TAG, "SSID:%s", ssid);
        ESP_LOGI(TAG, "PASSWORD:%s", password);
        if (evt->type == SC_TYPE_ESPTOUCH_V2)
        {
            ESP_ERROR_CHECK(esp_smartconfig_get_rvd_data(rvd_data, sizeof(rvd_data)));
            ESP_LOGI(TAG, "RVD_DATA:");
            for (int i = 0; i < 33; i++)
            {
                printf("%02x ", rvd_data[i]);
            }
            printf("\n");
        }

        ESP_ERROR_CHECK(esp_wifi_disconnect());
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
        esp_wifi_connect();
    }
    else if (event_base == SC_EVENT && event_id == SC_EVENT_SEND_ACK_DONE)
    {
        xEventGroupSetBits(s_wifi_event_group, ESPTOUCH_DONE_BIT);
    }
}
/**
 * @brief this is an exemple of a callback that you can setup in your own app to get notified of wifi manager event.
 */
void cb_connection_ok(void *pvParameter)
{
    ip_event_got_ip_t *param = (ip_event_got_ip_t *)pvParameter;

    /* transform IP to human readable string */
    char str_ip[16];
    esp_ip4addr_ntoa(&param->ip_info.ip, str_ip, IP4ADDR_STRLEN_MAX);

    ESP_LOGI(TAG, "I have a connection and my IP is %s!", str_ip);
    xTaskCreate(time_init_task, "time", 2048, NULL, 5, NULL);
}

static void initialise_wifi(void)
{
    // ESP_ERROR_CHECK(esp_netif_init());
    // s_wifi_event_group = xEventGroupCreate();
    // ESP_ERROR_CHECK(esp_event_loop_create_default());
    // esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    // assert(sta_netif);

    // wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    // ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    // ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));
    // ESP_ERROR_CHECK(esp_event_handler_register(SC_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));

    // ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    // ESP_ERROR_CHECK(esp_wifi_start());

    /* start the wifi manager */
    wifi_manager_start();

    /* register a callback as an example to how you can integrate your code with the wifi manager */
    wifi_manager_set_callback(WM_EVENT_STA_GOT_IP, &cb_connection_ok);
}

static void smartconfig_example_task(void *parm)
{
    EventBits_t uxBits;
    wifi_config_t myconfig = {0};

    esp_wifi_get_config(ESP_IF_WIFI_STA, &myconfig);
    if (strlen((char *)myconfig.sta.ssid) > 0)
    {
        ESP_LOGI(TAG, "already set SSID:%s,connect now!...", myconfig.sta.ssid);
        esp_wifi_connect();
    }
    else
    {
        ESP_ERROR_CHECK(esp_smartconfig_set_type(SC_TYPE_ESPTOUCH));
        smartconfig_start_config_t cfg = SMARTCONFIG_START_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_smartconfig_start(&cfg));
    }
    while (1)
    {
        uxBits = xEventGroupWaitBits(s_wifi_event_group, CONNECTED_BIT | ESPTOUCH_DONE_BIT, true, false, portMAX_DELAY);
        if (uxBits & CONNECTED_BIT)
        {
            ESP_LOGI(TAG, "WiFi Connected to ap");
            mqtt_app_start();
            xTaskCreate(time_init_task, "time", 2048, NULL, 5, NULL);
            xTaskCreate(tempsensor_example, "temp", 4096, NULL, 5, NULL);
        }
        if (uxBits & ESPTOUCH_DONE_BIT)
        {
            ESP_LOGI(TAG, "smartconfig over");
            esp_smartconfig_stop();
            vTaskDelete(NULL);
        }
    }
}

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0)
    {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

/*
 * @brief Event handler registered to receive MQTT events
 *
 *  This function is called by the MQTT client event loop.
 *
 * @param handler_args user data registered to the event.
 * @param base Event base for the handler(always MQTT Base in this example).
 * @param event_id The id for the received event.
 * @param event_data The data for the event, esp_mqtt_event_handle_t.
 */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id)
    {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        msg_id = esp_mqtt_client_publish(client, "/topic/qos1", "data_3", 0, 1, 0);
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);

        msg_id = esp_mqtt_client_subscribe(client, "/topic/qos0", 0);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

        msg_id = esp_mqtt_client_subscribe(client, "/topic/qos1", 1);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

        msg_id = esp_mqtt_client_unsubscribe(client, "/topic/qos1");
        ESP_LOGI(TAG, "sent unsubscribe successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        // printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        // printf("DATA=%.*s\r\n", event->data_len, event->data);
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT)
        {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno", event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
        }
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

static void mqtt_app_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .uri = CONFIG_BROKER_URL,
    };
    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(mqtt_client);
}

void app_main(void)
{
    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("MQTT_CLIENT", ESP_LOG_VERBOSE);
    esp_log_level_set("MQTT_EXAMPLE", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_BASE", ESP_LOG_VERBOSE);
    esp_log_level_set("esp-tls", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
    esp_log_level_set("OUTBOX", ESP_LOG_VERBOSE);

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    // ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(i2cdev_init());

    memset(&dev, 0, sizeof(sht3x_t));

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    // xTaskCreate(tempsensor_example, "temp", 4096, NULL, 5, NULL);
    xTaskCreate(read_temperature_humidity_task, "temp", 4096, NULL, 5, NULL);
    xTaskCreate(blink_task, "blink", configMINIMAL_STACK_SIZE, NULL, 5, NULL);
    xTaskCreate(button_init_task, "button", configMINIMAL_STACK_SIZE, NULL, 5, NULL);
    initialise_wifi();
    // ESP_ERROR_CHECK(example_connect());
    // mqtt_app_start();
    ESP_LOGI(TAG, "RUN TO HERE");
}
