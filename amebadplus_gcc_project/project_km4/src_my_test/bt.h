#ifndef __BT_H__
#define __BT_H__

#include <stdint.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <osif.h>

#include <bt_api_config.h>
#include <rtk_bt_def.h>
#include <rtk_bt_common.h>
#include <rtk_bt_device.h>
#include "rtk_bt_vendor.h"
#include <rtk_bt_gap.h>
#include <rtk_bt_le_gap.h>
#include <rtk_bt_att_defs.h>
#include <rtk_bt_gatts.h>
#include <rtk_bt_gattc.h>

#include <rtk_service_config.h>
#include <rtk_bas.h>
#include <rtk_hrs.h>
#include <rtk_simple_ble_service.h>
#include <rtk_dis.h>
#include <rtk_ias.h>
#include <rtk_hids_kb.h>
#include <rtk_gls.h>
#include <rtk_long_uuid_service.h>
#include <bt_utils.h>

typedef struct
{
    uint8_t srv_id;
    uint8_t attr_index;
} ble_gatts_attr_t;

typedef enum
{
    my_BLE_NOTIFY = 0,
    my_BLE_INDICATE = 1,
    my_BLE_WRITE_WITHOUT_RESP = 2,
    my_BLE_WRITE = 3,
} my_ble_send_type_t;

typedef struct
{
    my_ble_send_type_t type;
    uint16_t conn;
    ble_gatts_attr_t *handle;
    uint8_t *data;
    uint16_t length;
} my_ble_send_t;

typedef struct
{
    rtk_bt_le_scan_type_t type;

    uint16_t interval;

    uint16_t window;
    /** Own_Address_Type */
    rtk_bt_le_addr_type_t own_addr_type;
    /** Scanning_Filter_Policy */
    rtk_bt_le_scan_filter_t filter_policy;
    /** Scanning_Filter_Duplicated_Option */
    uint8_t duplicate_opt;
} my_ble_scan_param_t;

typedef struct
{

} my_ble_mode_t;

typedef struct
{
    uint16_t min_interval;
    uint16_t max_interval;
    uint16_t latency;
    uint16_t timeout;
} my_ble_config_t;

// Start of Selection
typedef struct
{
    uint8_t type;
    uint8_t val[16]; // 128位UUID
} my_ble_uuid_t;

typedef enum
{
    my_BLE_UUID_TYPE_16 = 0,
    my_BLE_UUID_TYPE_128 = 1,
} my_ble_uuid_type_t;

typedef struct
{
    uint8_t val[2];
} my_ble_uuid_16_t;

typedef struct
{
    uint8_t val[16];
} my_ble_uuid_128_t;

typedef struct
{
    my_ble_uuid_t *server_uuid[2];
    my_ble_uuid_t *tx_char_uuid[2];
    my_ble_uuid_t *rx_char_uuid[2];
    uint8_t srv_id;
    uint8_t attr_index;
} my_ble_default_server_t;

typedef struct
{
    ble_gatts_attr_t *tx_char_handle[2];
    ble_gatts_attr_t *rx_char_handle[2];
} my_ble_default_handle_t;

typedef uint16_t my_ble_conn_t;

int32_t my_ble_init(my_ble_mode_t mode, const my_ble_config_t *config);
int32_t my_ble_adv_start(const uint8_t *ad, uint16_t ad_len, const uint8_t *sd, uint16_t sd_len);
int32_t my_ble_adv_stop(void);
int32_t my_ble_get_mac(uint8_t *mac);
int32_t my_ble_set_mac(const uint8_t *mac);
int32_t my_ble_scan(const my_ble_scan_param_t *cfg);
int32_t my_ble_scan_stop(void);
int32_t my_ble_set_name(const char *name);
int32_t ble_gatts_add_blufi_svcs(my_ble_default_server_t *cfg, my_ble_default_handle_t *handle);

#endif
