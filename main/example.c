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
#include "protocol_examples_common.h"
/*
smartconfig
*/
#include "esp_smartconfig.h"
#include "freertos/event_groups.h"
// #include "esp_wpa2.h"
/*
定时器
*/
#include "driver/timer.h"
/*
ADC
*/
#include "driver/adc.h"
#include "driver/gpio.h"
/*
I2C
*/
#include "sht21.h"
/*
button
*/
#include "my_button.h"
/*
RMT
*/
#include "driver/rmt.h"
#include "led_strip.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"

#define TIMER_SCALE (TIMER_BASE_CLK / 16) // convert counter value to seconds

#define ONENET_MqttServer "mqtts.heclouds.com"

#define ONENET_clientld "esp32-device1" // name
#define ONENET_UserName "493136"        // ID
#define ONENET_Password "version=2018-10-31&res=products%2F493136%2Fdevices%2Fesp32-device1&et=1741490120&method=md5&sign=wscxn0OWnKxbxQ7mae3CLQ%3D%3D"

//设备上传数据的post主题
#define ONENET_TOPIC_PROP_POST "$sys/" ONENET_UserName "/" ONENET_clientld "/dp/post/json"

#define ONENET_TOPIC_DP_Publish "$sys/493136/esp32-device1/dp/post/json"
#define ONENET_TOPIC_DP_Sub "$sys/493136/esp32-device1/dp/post/json/+"
#define ONENET_TOPIC_CMD_Sub "$sys/493136/esp32-device1/cmd/#"

//这是post上传数据使用的模板
#define ONENET_POST_BODY_FORMAT "{\"id\":123,\"dp\":%s}"

static EventGroupHandle_t s_wifi_event_group;

static const int CONNECTED_BIT = BIT0;
static const int ESPTOUCH_DONE_BIT = BIT1;
static const char *TAG = "MQTT";
static const char *TAG2 = "smartconfig";

// static EventGroupHandle_t s_cmd_event_group;
static const int RED_BIT = BIT3;
static const int BLUE_BIT = BIT4;

bool send_cycle = false;

typedef struct
{
    int timer_group;
    int timer_idx;
    int alarm_interval;
    bool auto_reload;
} example_timer_info_t;

#define RMT_TX_NUM 8 // WS2812 control pin
#define RMT_TX_CHANNEL RMT_CHANNEL_0
#define EXAMPLE_CHASE_SPEED_MS (10)
#define LED_STRIP_NUM 1
#define COLOR_RED 9
#define COLOR_GREE 8
#define COLOR_BULE 10
#define COLOR_PURPLE 11
#define COLOR_ORANGE 12

#define Lamp_Open 13
#define lamp_close 14
#define lamp_speed_Up 15
#define lamp_speed_down 16

#define ligth_down 18
#define ligth_up 19
struct WS2812_COLOR
{
    uint8_t lamp;
    uint8_t ligth_rank;
    uint8_t lamp_speed;
    uint32_t red;
    uint32_t green;
    uint32_t blue;
};

static led_strip_t *strip;

struct WS2812_COLOR WS2812_RGB;

/**
 * @brief A sample structure to pass events from the timer ISR to task
 *
 */
typedef struct
{
    example_timer_info_t info;
    uint64_t timer_counter_value;
} example_timer_event_t;

static void smartconfig_example_task(void *parm);
static void led_cmd_task(void *parm);

void RGB16for10(struct WS2812_COLOR *RGB, uint32_t reb_16)
{
    uint32_t rgb_16 = reb_16;
    RGB->blue = rgb_16 & 0Xff;

    rgb_16 = rgb_16 >> 8;
    RGB->green = rgb_16 & 0xff;

    rgb_16 = rgb_16 >> 8;

    RGB->red = rgb_16 & 0xff;
}

void set_rgb(uint32_t rgb_24bit, uint8_t ligth_rank)
{
    RGB16for10(&WS2812_RGB, rgb_24bit);
    ligth_rank = 21 - ligth_rank;
    for (int i = 0; i < LED_STRIP_NUM; i++)
    {
        strip->set_pixel(strip, i, WS2812_RGB.red / ligth_rank, WS2812_RGB.green / ligth_rank, WS2812_RGB.blue / ligth_rank);
    }

    strip->refresh(strip, 10);
}

void init_led()
{
    rmt_config_t config = RMT_DEFAULT_CONFIG_TX(RMT_TX_NUM, RMT_TX_CHANNEL);
    // set counter clock to 40MHz
    config.clk_div = 2;
    ESP_ERROR_CHECK(rmt_config(&config));
    ESP_ERROR_CHECK(rmt_driver_install(config.channel, 0, 0));

    // install ws2812 driver
    led_strip_config_t strip_config = LED_STRIP_DEFAULT_CONFIG(1, (led_strip_dev_t)config.channel);
    strip = led_strip_new_rmt_ws2812(&strip_config);
    if (!strip)
    {
        ESP_LOGE(TAG, "install WS2812 driver failed");
    }
    // Clear LED strip (turn off all LEDs)
    ESP_ERROR_CHECK(strip->clear(strip, 100));
}

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0)
    {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

static int single_read(void *arg)
{
    int adc1_reading[1] = {0};
    adc1_reading[0] = adc1_get_raw(ADC1_CHANNEL_0);
    // vout = (adc1_reading[0] * 2500.00)/4095.00;
    // ESP_LOGI(TAG_CH[0], "%x vout mv is %f", adc1_reading[0],vout);
    return adc1_reading[0];
}

static bool IRAM_ATTR system_timer_callback(void *arg)
{
    BaseType_t high_task_awoken = pdFALSE;
    send_cycle = true;
    return high_task_awoken == pdTRUE; // return whether we need to yield at the end of ISR
}

static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        xTaskCreate(smartconfig_example_task, "smartconfig_example_task", 4096, NULL, 3, NULL);
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        esp_wifi_connect();
        xEventGroupClearBits(s_wifi_event_group, CONNECTED_BIT);
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        xEventGroupSetBits(s_wifi_event_group, CONNECTED_BIT);
    }
    else if (event_base == SC_EVENT && event_id == SC_EVENT_SCAN_DONE)
    {
        ESP_LOGI(TAG2, "Scan done");
    }
    else if (event_base == SC_EVENT && event_id == SC_EVENT_FOUND_CHANNEL)
    {
        ESP_LOGI(TAG2, "Found channel");
    }
    else if (event_base == SC_EVENT && event_id == SC_EVENT_GOT_SSID_PSWD)
    {
        ESP_LOGI(TAG2, "Got SSID and password");

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
        ESP_LOGI(TAG2, "SSID:%s", ssid);
        ESP_LOGI(TAG2, "PASSWORD:%s", password);
        if (evt->type == SC_TYPE_ESPTOUCH_V2)
        {
            ESP_ERROR_CHECK(esp_smartconfig_get_rvd_data(rvd_data, sizeof(rvd_data)));
            ESP_LOGI(TAG2, "RVD_DATA:");
            for (int i = 0; i < 33; i++)
            {
                printf("%02x ", rvd_data[i]);
            }
            printf("\n");
        }

        ESP_ERROR_CHECK(esp_wifi_disconnect());
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
        printf("ready to connect!\r\n");
        esp_wifi_connect();
    }
    else if (event_base == SC_EVENT && event_id == SC_EVENT_SEND_ACK_DONE)
    {
        xEventGroupSetBits(s_wifi_event_group, ESPTOUCH_DONE_BIT);
    }
}

static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    char cmdbuf[20];
    // your_context_t *context = event->context;
    switch (event->event_id)
    {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        // msg_id = esp_mqtt_client_publish(client, "/topic/qos1", "data_3", 0, 1, 0);
        // ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);

        msg_id = esp_mqtt_client_subscribe(client, ONENET_TOPIC_DP_Sub, 0);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

        msg_id = esp_mqtt_client_subscribe(client, ONENET_TOPIC_CMD_Sub, 1);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

        // msg_id = esp_mqtt_client_unsubscribe(client, "/topic/qos1");
        // ESP_LOGI(TAG, "sent unsubscribe successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        // esp_mqtt_client_start(client);
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        // msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
        // ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);

        sprintf(cmdbuf, "%.*s", event->data_len, event->data);
        if (strstr(cmdbuf, "：ledred"))
        {
            xEventGroupSetBits(s_wifi_event_group, RED_BIT);
        }
        else if (strstr(cmdbuf, "：ledblue"))
        {
            xEventGroupSetBits(s_wifi_event_group, BLUE_BIT);
        }
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
    return ESP_OK;
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    mqtt_event_handler_cb(event_data);
}

static void mqtt_app_start(void)
{
    int msg_id;
    int adc_value;
    float adc_mv;
    /* ADC */
    adc1_config_width(ADC_WIDTH_BIT_DEFAULT);
    adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11);
    /* */
    esp_mqtt_client_config_t mqtt_cfg = {
        // .uri = ONENET_MqttServer,
        .host = "mqtts.heclouds.com",
        // .uri = "mqtt://broker.emqx.io",
        .client_id = ONENET_clientld,
        .username = ONENET_UserName,
        .password = ONENET_Password,
        .port = 1883,
    };
#if CONFIG_BROKER_URL_FROM_STDIN
    char line[128];

    if (strcmp(mqtt_cfg.uri, "FROM_STDIN") == 0)
    {
        int count = 0;
        printf("Please enter url of mqtt broker\n");
        while (count < 128)
        {
            int c = fgetc(stdin);
            if (c == '\n')
            {
                line[count] = '\0';
                break;
            }
            else if (c > 0 && c < 127)
            {
                line[count] = c;
                ++count;
            }
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
        mqtt_cfg.uri = line;
        printf("Broker url: %s\n", line);
    }
    else
    {
        ESP_LOGE(TAG, "Configuration mismatch: wrong broker url");
        abort();
    }
#endif /* CONFIG_BROKER_URL_FROM_STDIN */

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_start(client);

    // char *adc_head ="my adc value is:";
    // char *adc_tail ="mv";
    // char adc_publish[25] = {0};

    // int thread;
    float T, H;

    char jsonBuf[100];
    char databuf[80];

    while (1)
    {
        if (send_cycle)
        {
            send_cycle = false;
            adc_value = single_read(NULL);
            adc_mv = (adc_value * 2500.00) / 4095.00;

            SHT2X_THMeasure(I2C_MASTER_NUM);
            // if(thread == ESP_ERR_TIMEOUT){}
            // else{
            T = (getTemperature() / 100.0);
            H = (getHumidity() / 100.0);
            printf("this is my th test, %4.2f C\r\n%4.2f %%\r\n", T, H);
            // }

            sprintf(databuf, "{ \"tem\":[{\"v\":%.2f}] ,\"hum\":[{\"v\":%.2f}] ,\"adc\":[{\"v\":%.2f}] }", T, H, adc_mv);
            sprintf(jsonBuf, ONENET_POST_BODY_FORMAT, databuf);
            // sprintf(adc_publish,"%s %.2f %s",adc_head,adc_mv,adc_tail);
            msg_id = esp_mqtt_client_publish(client, ONENET_TOPIC_DP_Publish, jsonBuf, 0, 1, 0);
            ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
        }

        vTaskDelay(1);
    }
}

static void initialise_wifi(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    s_wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    /*
    just for test  ESP_ERR_WIFI_NOT_INIT: WiFi is not initialized by esp_wifi_init
    esp_wifi_restore 使用需要在 esp_wifi_init 之后
    */
    // esp_wifi_restore();

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(SC_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    xTaskCreate(led_cmd_task, "led_cmd_task", 2048, NULL, 2, NULL);
}

/**
 * @brief Initialize selected timer of timer group
 *
 * @param group Timer Group number, index from 0
 * @param timer timer ID, index from 0
 * @param auto_reload whether auto-reload on alarm event
 * @param timer_interval_sec interval of alarm
 */
static void example_tg_timer_init(int group, int timer, bool auto_reload, int timer_interval_sec)
{
    /* Select and initialize basic parameters of the timer */
    timer_config_t config = {
        .divider = 16,
        .counter_dir = TIMER_COUNT_UP,
        .counter_en = TIMER_PAUSE,
        .alarm_en = TIMER_ALARM_EN,
        .auto_reload = auto_reload,
    }; // default clock source is APB
    timer_init(group, timer, &config);

    /* Timer's counter will initially start from value below.
       Also, if auto_reload is set, this value will be automatically reload on alarm */
    timer_set_counter_value(group, timer, 0);

    /* Configure the alarm value and the interrupt on alarm. */
    timer_set_alarm_value(group, timer, timer_interval_sec * (TIMER_BASE_CLK / 16));
    timer_enable_intr(group, timer);

    example_timer_info_t *timer_info = calloc(1, sizeof(example_timer_info_t));
    timer_info->timer_group = group;
    timer_info->timer_idx = timer;
    timer_info->auto_reload = auto_reload;
    timer_info->alarm_interval = timer_interval_sec;
    timer_isr_callback_add(group, timer, system_timer_callback, timer_info, 0);

    timer_start(group, timer);
}

static void smartconfig_example_task(void *parm)
{
    EventBits_t uxBits;

    wifi_config_t myconfig = {0};
    esp_wifi_get_config(ESP_IF_WIFI_STA, &myconfig);
    if (strlen((char *)myconfig.sta.ssid) > 0)
    {
        ESP_LOGI(TAG2, "already set SSID:%s,connect now!...", myconfig.sta.ssid);
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
            ESP_LOGI(TAG2, "WiFi Connected to ap");
            example_tg_timer_init(TIMER_GROUP_0, TIMER_0, true, 5);
            vTaskDelete(NULL);
        }
        if (uxBits & ESPTOUCH_DONE_BIT)
        {
            ESP_LOGI(TAG2, "smartconfig over");
            esp_smartconfig_stop();
            example_tg_timer_init(TIMER_GROUP_0, TIMER_0, true, 5);
            vTaskDelete(NULL);
        }
    }
}

static void led_cmd_task(void *parm)
{
    EventBits_t uxBits;
    while (1)
    {
        uxBits = xEventGroupWaitBits(s_wifi_event_group, RED_BIT | BLUE_BIT, true, false, portMAX_DELAY);
        if (uxBits & RED_BIT)
        {
            ESP_LOGI(TAG, "LED Turn RED");
            set_rgb(0x00FF00, WS2812_RGB.ligth_rank);
        }
        if (uxBits & BLUE_BIT)
        {
            ESP_LOGI(TAG, "LED Turn BLUE");
            set_rgb(0X0000ff, WS2812_RGB.ligth_rank);
        }
        // set_rgb(0Xff0000, WS2812_RGB.ligth_rank);//绿色
        // set_rgb(0x00FF00, WS2812_RGB.ligth_rank); //红色
        // set_rgb(0X0000ff, WS2812_RGB.ligth_rank);//蓝色
        // set_rgb(0XF05308, WS2812_RGB.ligth_rank); //黄绿色
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("MQTT_CLIENT", ESP_LOG_VERBOSE);
    esp_log_level_set("MQTT_EXAMPLE", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_TCP", ESP_LOG_VERBOSE);

    esp_log_level_set("TRANSPORT_SSL", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
    esp_log_level_set("OUTBOX", ESP_LOG_VERBOSE);

    ESP_ERROR_CHECK(nvs_flash_init());
    // ESP_ERROR_CHECK(esp_netif_init());
    // ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    // ESP_ERROR_CHECK(example_connect());

    ESP_ERROR_CHECK(i2c_master_init());

    button_start();

    init_led();

    initialise_wifi();

    mqtt_app_start();
}
