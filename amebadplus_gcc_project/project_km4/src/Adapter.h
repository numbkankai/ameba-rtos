/*
 * @Author: ThankYou
 * @Date: 2024-12-24 11:17:41
 * @LastEditors: ThankYou
 * @LastEditTime: 2024-12-26 16:19:56
 * @FilePath: /ameba-rtos/amebadplus_gcc_project/project_km4/src/Adapter.h
 * @Description:
 *
 * Copyright (c) 2024 by ThankYou, All Rights Reserved.
 */
#ifndef ADAPTER_h
#define ADAPTER_h

#include <stdint.h>
#include "kv.h"

#include <stdbool.h>
#include "os_wrapper.h"
#include "log.h"
#ifdef __cplusplus
extern "C"
{
#endif

#define CONFIG_ESP_WIFI_SSID "YE_HCW10_ABCD"
#define CONFIG_ESP_WIFI_PASSWORD "12345678"
#define pdFALSE 0
#define portMAX_DELAY 100
#define WIFI_MODE_STA 0
#define WIFI_IF_STA 0
#define BIT0 0
#define BIT1 1

#define ESP_ERR_NVS_BASE 0x1100
#define ESP_ERR_NVS_NO_FREE_PAGES (ESP_ERR_NVS_BASE + 0x0d)
#define ESP_ERR_NVS_NEW_VERSION_FOUND (ESP_ERR_NVS_BASE + 0x10)
#define ESP_ERR_NVS_NOT_FOUND (ESP_ERR_NVS_BASE + 0x02)

#define IPSTR "%d.%d.%d.%d"
#define esp_ip4_addr_get_byte(ipaddr, idx) (((const uint8_t *)(&(ipaddr)->addr))[idx])
#define esp_ip4_addr1(ipaddr) esp_ip4_addr_get_byte(ipaddr, 0)
#define esp_ip4_addr2(ipaddr) esp_ip4_addr_get_byte(ipaddr, 1)
#define esp_ip4_addr3(ipaddr) esp_ip4_addr_get_byte(ipaddr, 2)
#define esp_ip4_addr4(ipaddr) esp_ip4_addr_get_byte(ipaddr, 3)

#define esp_ip4_addr1_16(ipaddr) ((uint16_t)esp_ip4_addr1(ipaddr))
#define esp_ip4_addr2_16(ipaddr) ((uint16_t)esp_ip4_addr2(ipaddr))
#define esp_ip4_addr3_16(ipaddr) ((uint16_t)esp_ip4_addr3(ipaddr))
#define esp_ip4_addr4_16(ipaddr) ((uint16_t)esp_ip4_addr4(ipaddr))

#define ESP_LOGI RTK_LOGI
#define WIFI_INIT_CONFIG_DEFAULT() {0}

#define IP2STR(ipaddr) esp_ip4_addr1_16(ipaddr), \
                       esp_ip4_addr2_16(ipaddr), \
                       esp_ip4_addr3_16(ipaddr), \
                       esp_ip4_addr4_16(ipaddr)

#ifndef unlikely
#define unlikely(x) (x)
#endif

    typedef int esp_err_t;
    typedef uint32_t nvs_handle_t;
    typedef uint32_t nvs_handle_t;
#define ESP_OK 0    /*!< esp_err_t value indicating success (no error) */
#define ESP_FAIL -1 /*!< Generic esp_err_t code indicating failure */
#define ESP_ERROR_CHECK(x)                                                          \
    do                                                                              \
    {                                                                               \
        esp_err_t err_rc_ = (x);                                                    \
        if (unlikely(err_rc_ != ESP_OK))                                            \
        {                                                                           \
            printf("Error: %d, File: %s, Line: %d, Function: %s, Expression: %s\n", \
                   err_rc_, __FILE__, __LINE__, __ASSERT_FUNC, #x);                 \
        }                                                                           \
    } while (0)

    typedef enum
    {
        WIFI_EVENT,
        IP_EVENT,
    } esp_event_base_t;

    enum
    {
        ESP_EVENT_ANY_ID,
        WIFI_EVENT_STA_START,
        WIFI_EVENT_STA_DISCONNECTED,
        IP_EVENT_STA_GOT_IP,
    } event_id;

    struct esp_ip6_addr
    {
        uint32_t addr[4]; /*!< IPv6 address */
        uint8_t zone;     /*!< zone ID */
    };
    struct esp_ip4_addr
    {
        uint32_t addr; /*!< IPv4 address */
    };

    typedef struct esp_ip4_addr esp_ip4_addr_t;
    typedef struct esp_ip6_addr esp_ip6_addr_t;

    typedef struct _ip_addr
    {
        union
        {
            esp_ip6_addr_t ip6; /*!< IPv6 address type */
            esp_ip4_addr_t ip4; /*!< IPv4 address type */
        } u_addr;               /*!< IP address union */
        uint8_t type;           /*!< ipaddress type */
    } esp_ip_addr_t;

    typedef struct
    {
        esp_ip4_addr_t ip;      /**< Interface IPV4 address */
        esp_ip4_addr_t netmask; /**< Interface IPV4 netmask */
        esp_ip4_addr_t gw;      /**< Interface IPV4 gateway address */
    } esp_netif_ip_info_t;

    struct esp_netif_obj;
    typedef struct esp_netif_obj esp_netif_t;
    typedef struct
    {
        esp_netif_t *esp_netif;      /*!< Pointer to corresponding esp-netif object */
        esp_netif_ip_info_t ip_info; /*!< IP address, netmask, gatway IP address */
        bool ip_changed;             /*!< Whether the assigned IP has changed or not */
    } ip_event_got_ip_t;

    typedef struct
    {

    } wifi_init_config_t;

    typedef enum
    {
        WIFI_AUTH_OPEN = 0,                               /**< authenticate mode : open */
        WIFI_AUTH_WEP,                                    /**< authenticate mode : WEP */
        WIFI_AUTH_WPA_PSK,                                /**< authenticate mode : WPA_PSK */
        WIFI_AUTH_WPA2_PSK,                               /**< authenticate mode : WPA2_PSK */
        WIFI_AUTH_WPA_WPA2_PSK,                           /**< authenticate mode : WPA_WPA2_PSK */
        WIFI_AUTH_ENTERPRISE,                             /**< authenticate mode : WiFi EAP security */
        WIFI_AUTH_WPA2_ENTERPRISE = WIFI_AUTH_ENTERPRISE, /**< authenticate mode : WiFi EAP security */
        WIFI_AUTH_WPA3_PSK,                               /**< authenticate mode : WPA3_PSK */
        WIFI_AUTH_WPA2_WPA3_PSK,                          /**< authenticate mode : WPA2_WPA3_PSK */
        WIFI_AUTH_WAPI_PSK,                               /**< authenticate mode : WAPI_PSK */
        WIFI_AUTH_OWE,                                    /**< authenticate mode : OWE */
        WIFI_AUTH_WPA3_ENT_192,                           /**< authenticate mode : WPA3_ENT_SUITE_B_192_BIT */
        WIFI_AUTH_WPA3_EXT_PSK,                           /**< authenticate mode : WPA3_PSK_EXT_KEY */
        WIFI_AUTH_WPA3_EXT_PSK_MIXED_MODE,                /**< authenticate mode: WPA3_PSK + WPA3_PSK_EXT_KEY */
        WIFI_AUTH_MAX
    } wifi_auth_mode_t;

    typedef enum
    {
        NVS_READONLY, /*!< Read only */
        NVS_READWRITE /*!< Read and write */
    } nvs_open_mode_t;

    typedef struct
    {
        int8_t rssi;               /**< The minimum rssi to accept in the fast scan mode */
        wifi_auth_mode_t authmode; /**< The weakest authmode to accept in the fast scan mode
                                        Note: Incase this value is not set and password is set as per WPA2 standards(password len >= 8), it will be defaulted to WPA2 and device won't connect to deprecated WEP/WPA networks. Please set authmode threshold as WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK to connect to WEP/WPA networks */
    } wifi_scan_threshold_t;
    typedef struct
    {
        bool capable;  /**< Deprecated variable. Device will always connect in PMF mode if other device also advertizes PMF capability. */
        bool required; /**< Advertizes that Protected Management Frame is required. Device will not associate to non-PMF capable devices. */
    } wifi_pmf_config_t;
    typedef struct
    {
        uint8_t ssid[32];     /**< SSID of target AP. */
        uint8_t password[64]; /**< Password of target AP. */

        wifi_scan_threshold_t threshold; /**< When scan_threshold is set, only APs which have an auth mode that is more secure than the selected auth mode and a signal stronger than the minimum RSSI will be used. */
        wifi_pmf_config_t pmf_cfg;       /**< Configuration for Protected Management Frame. Will be advertised in RSN Capabilities in RSN IE. */
    } wifi_sta_config_t;

    typedef struct
    {
        wifi_sta_config_t sta;
    } wifi_config_t;

    int esp_wifi_connect(void);
    int gpio_set_level(uint8_t u8CPin, uint8_t u8COnOff);
    int esp_wifi_stop(void);
    int esp_wifi_deinit(void);
    int esp_netif_init(void);
    int esp_event_loop_create_default(void);
    int esp_wifi_init(wifi_init_config_t *cfg);
    int esp_netif_create_default_wifi_sta(void);

    typedef void (*esp_event_handler_t)(void *event_handler_arg,
                                        esp_event_base_t event_base,
                                        int32_t event_id,
                                        void *event_data); /**< function called when an event is posted to the queue */
    typedef void *esp_event_handler_instance_t;            /**< context identifying an instance of a registered event handler */
    esp_err_t esp_event_handler_instance_register(esp_event_base_t event_base,
                                                  int32_t event_id,
                                                  esp_event_handler_t event_handler,
                                                  void *event_handler_arg,
                                                  esp_event_handler_instance_t *context);

    esp_err_t nvs_flash_init(void);
    esp_err_t nvs_open(const char *namespace_name, nvs_open_mode_t open_mode, nvs_handle_t *out_handle);
    esp_err_t nvs_get_blob(nvs_handle_t c_handle, const char *key, void *out_value, size_t *length);
    esp_err_t nvs_set_blob(nvs_handle_t c_handle, const char *key, const void *value, size_t length);
    esp_err_t nvs_flash_erase(void);
    void lnkSync_init(void);
    void socketClient_Init(void);

#ifdef __cplusplus
}
#endif
#endif // END ADAPTER_h