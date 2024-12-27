/*
 * @Author: ThankYou
 * @Date: 2024-12-24 11:17:04
 * @LastEditors: ThankYou
 * @LastEditTime: 2024-12-26 16:19:50
 * @FilePath: /ameba-rtos/amebadplus_gcc_project/project_km4/src/Adapter.c
 * @Description:
 *
 * Copyright (c) 2024 by ThankYou, All Rights Reserved.
 */
#include "Adapter.h"
#include <platform_autoconf.h>
#include "platform_stdlib.h"
#include "basic_types.h"

#include "wifi_conf.h"
#include "wifi_ind.h"
#include "lwip_netconf.h"
#include "os_wrapper.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "ameba_soc.h"
#include "gpio_ext.h"
#include "device.h"
#include "gpio_api.h" // mbed
#include "main.h"
#include "os_wrapper.h"
#include <stdio.h>

// char *test_ssid = "QCA9531_18c0";
char *test_ssid = "YE_HCW10_ABCD";
// char *test_ssid = "Xiaomi_5AFD_5G";
char *test_password = "12345678";
u32 secure_type = RTW_SECURITY_WPA2_AES_PSK; /*Just distinguish between WEP, TKIP/WPA/WPA2/WPA3 can use the same secure_type*/
int esp_wifi_connect(void)
{
#if 0
    struct _rtw_network_info_t connect_param = {0};
    /*Connect parameter set*/
    memcpy(connect_param.ssid.val, test_ssid, strlen(test_ssid));
    connect_param.ssid.len = strlen(test_ssid);
    connect_param.password = (unsigned char *)test_password;
    connect_param.password_len = strlen(test_password);
    connect_param.security_type = secure_type;
    wifi_connect(&connect_param, 1);
#endif
    return 0;
}

#define GPIO_LED_G 16
#define GPIO_LED_Y 17
int gpio_set_level(uint8_t u8CPin, uint8_t u8COnOff)
{
    gpio_t gpio_led;
    if (GPIO_LED_G == u8CPin)
    {
        gpio_init(&gpio_led, GPIO_LED_PIN);
    }
    else if (GPIO_LED_Y == u8CPin)
    {
        gpio_init(&gpio_led, GPIO_LED_PIN);
    }
    gpio_dir(&gpio_led, PIN_OUTPUT); // Direction: Output
    gpio_mode(&gpio_led, PullNone);  // No pull
    gpio_direct_write(&gpio_led, u8COnOff);
    return 0;
}

int esp_wifi_stop(void)
{
    return 0;
}
int esp_wifi_deinit(void)
{
    return 0;
}
int esp_netif_init(void)
{
    return 0;
}
int esp_event_loop_create_default(void)
{
    return 0;
}
int esp_wifi_init(wifi_init_config_t *cfg)
{
    printf("%p", cfg);
    return 0;
}

int esp_netif_create_default_wifi_sta(void)
{
    return 0;
}

static void exampe_wifi_join_status_event_hdl(char *buf, int buf_len, int flags, void *userdata)
{
    UNUSED(buf_len);
    UNUSED(userdata);
    u8 join_status = (u8)flags;
    struct rtw_event_join_fail_info_t *fail_info = (struct rtw_event_join_fail_info_t *)buf;
    struct rtw_event_disconn_info_t *disconn_info = (struct rtw_event_disconn_info_t *)buf;

    void led_wifi_isConnected(bool status);
    if (join_status == RTW_JOINSTATUS_STARTING)
    {
        RTK_LOGI(TAG, "Join start\n");
        led_wifi_isConnected(false);
    }

    if (join_status == RTW_JOINSTATUS_SUCCESS)
    {
        RTK_LOGI(TAG, "Join success\n");
        led_wifi_isConnected(true);
        return;
    }

    /*Get join fail reason*/
    if (join_status == RTW_JOINSTATUS_FAIL)
    {                                                                     /*Include 4 way handshake but not include DHCP*/
        RTK_LOGI(TAG, "Join fail, reason = %d ", fail_info->fail_reason); /*definition in enum int*/
        led_wifi_isConnected(false);
        return;
    }

    /*Get disconnect reason*/
    if (join_status == RTW_JOINSTATUS_DISCONNECT)
    {
        RTK_LOGI(TAG, "Disconnect, reason = %d\n", disconn_info->disconn_reason);
        led_wifi_isConnected(false);
        return;
    }
}

esp_err_t esp_event_handler_instance_register(esp_event_base_t event_base,
                                              int32_t event_id,
                                              esp_event_handler_t event_handler,
                                              void *event_handler_arg,
                                              esp_event_handler_instance_t *context)
{
    if (WIFI_EVENT == event_base)
    {
        wifi_reg_event_handler(WIFI_EVENT_JOIN_STATUS, exampe_wifi_join_status_event_hdl, NULL);
    }
    else if (IP_EVENT == event_base)
    {
        wifi_reg_event_handler(WIFI_EVENT_JOIN_STATUS, exampe_wifi_join_status_event_hdl, NULL);
    }
}

void lnkSync_init(void)
{
}

esp_err_t nvs_flash_init(void)
{
    return (esp_err_t)rt_kv_init();
}

esp_err_t nvs_open(const char *namespace_name, nvs_open_mode_t open_mode, nvs_handle_t *out_handle)
{
    return 0;
}

esp_err_t nvs_get_blob(nvs_handle_t c_handle, const char *key, void *out_value, size_t *length)
{
    return rt_kv_get(key, out_value, *length);
}

esp_err_t nvs_set_blob(nvs_handle_t c_handle, const char *key, const void *value, size_t length)
{
    return rt_kv_set(key, value, length);
}

esp_err_t nvs_flash_erase(void)
{
    return 0;
}

void socketClient_Init(void)
{
}