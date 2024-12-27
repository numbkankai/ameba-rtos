/*  WiFi softAP Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>

#ifndef C_BW20_5G
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"
#else
#include "Adapter.h"
#endif

static EventGroupHandle_t s_wifi_event_group;

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1
#define NVS_CONFIG_KEY "NVS_CONFIG"
#define CONFIG_ESP_MAXIMUM_RETRY 5

static const char *TAG = "YEWiFi";

static struct
{
    enum
    {
        notInit = 0,
        notOpen,
        notData,
        dataReady,
    } status;
    nvs_handle_t handle;
    struct
    {
        char SSID[32];
        char PASSWORD[64];
    } wifi;
} nvsCFG;

static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    void led_wifi_isConnected(bool status);
    static int s_retry_num = 0;
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        s_retry_num = 0;
        esp_wifi_connect();
        led_wifi_isConnected(false);
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        if (s_retry_num < CONFIG_ESP_MAXIMUM_RETRY)
        {
            s_retry_num++;
            esp_wifi_connect();
            ESP_LOGI(TAG, "retry to connect to the AP");
        }
        else
        {
            s_retry_num = 0;
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        led_wifi_isConnected(false);
        ESP_LOGI(TAG, "connect to the AP fail");
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        led_wifi_isConnected(true);
        s_retry_num = 0;
    }
}

void wifi_init_sta(char *wifiSSID, char *wifiPassword)
{
    static bool firstCall = true;

    esp_wifi_stop();
    esp_wifi_deinit();

    if (firstCall)
    {
        s_wifi_event_group = xEventGroupCreate();
        ESP_ERROR_CHECK(esp_netif_init());
        ESP_ERROR_CHECK(esp_event_loop_create_default());
        esp_netif_create_default_wifi_sta();
    }
    xEventGroupClearBits(s_wifi_event_group, WIFI_FAIL_BIT);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    if (firstCall)
    {
        ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                            ESP_EVENT_ANY_ID,
                                                            &wifi_event_handler,
                                                            NULL,
                                                            NULL));
        ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                            IP_EVENT_STA_GOT_IP,
                                                            &wifi_event_handler,
                                                            NULL,
                                                            NULL));
    }

    firstCall = false;
    wifi_config_t wifi_config = {
        .sta = {
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,

            .pmf_cfg = {
                .capable = true,
                .required = false},
        },
    };

    strcpy((char *)wifi_config.sta.ssid, wifiSSID);
    strcpy((char *)wifi_config.sta.password, wifiPassword);
    if (strlen(wifiPassword) == 0)
    {
        ESP_LOGI(TAG, "Connect to '%s' without password.\n", wifiSSID);
        wifi_config.sta.threshold.authmode = WIFI_AUTH_OPEN;
    }
    else
    {
        ESP_LOGI(TAG, "Connect to '%s'.\n", wifiSSID);
    }
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    xEventGroupWaitBits(s_wifi_event_group,
                        WIFI_FAIL_BIT,
                        pdFALSE,
                        pdFALSE,
                        portMAX_DELAY);
}

static void wifi_do_connect(void *arg)
{
    while (1)
    {
        wifi_init_sta(CONFIG_ESP_WIFI_SSID, CONFIG_ESP_WIFI_PASSWORD);
        if (nvsCFG.status == dataReady)
        {
            wifi_init_sta(nvsCFG.wifi.SSID, nvsCFG.wifi.PASSWORD);
        }
    }
}
void app_setWifi(const char *SSID, const char *PASSWORD)
{
    do
    {
        if (strcmp(SSID, nvsCFG.wifi.SSID) != 0)
            break;
        if (strcmp(PASSWORD, nvsCFG.wifi.PASSWORD) != 0)
            break;
        return;
    } while (0);
    strcpy(nvsCFG.wifi.SSID, SSID);
    strcpy(nvsCFG.wifi.PASSWORD, PASSWORD);
    nvs_set_blob(nvsCFG.handle, NVS_CONFIG_KEY, &nvsCFG.wifi, sizeof(nvsCFG.wifi));
}
void app_main(void)
{
    void socketClient_Init(void);
    void lnkSync_init(void);
    void led_power_on(void);

    led_power_on();

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ret = nvs_open(NVS_CONFIG_KEY, NVS_READWRITE, &nvsCFG.handle);
    if (ret != ESP_OK)
        nvsCFG.status = notOpen;
    else
    {
        size_t size = sizeof(nvsCFG.wifi);
        ret = nvs_get_blob(nvsCFG.handle, NVS_CONFIG_KEY, &nvsCFG.wifi, &size);
        ESP_LOGI(TAG, "Reading wifi-config from NVS ... ");
        nvsCFG.status = notData;
        switch (ret)
        {
        case ESP_OK:
            printf("Done\n");
            nvsCFG.status = dataReady;
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            ESP_LOGI(TAG, "The value is not initialized yet!\n");
            break;
        default:
            ESP_LOGI(TAG, "Error (%s) reading!\n", esp_err_to_name(ret));
        }
    }
    xTaskCreate(&wifi_do_connect, "wifi_do_connect", 4096, NULL, 5, NULL);
    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");

    lnkSync_init();
    socketClient_Init();
}
