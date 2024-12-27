#include "usr_main.h"
#include <stdio.h>

/******************************************************************************
 *
 * Copyright(c) 2007 - 2018 Realtek Corporation. All rights reserved.
 *
 ******************************************************************************/

#include <platform_autoconf.h>
#include "platform_stdlib.h"
#include "basic_types.h"

#include "wifi_conf.h"
#include "wifi_ind.h"
#include "lwip_netconf.h"
#include "os_wrapper.h"

#include <stdlib.h>
#include <string.h>
#include <atcmd_service.h>
#include <bt_utils.h>
#include <atcmd_bt_impl.h>
#include <rtk_simple_ble_service.h>
#include "bt.h"
/********************************User configure**************************/
#define RECONNECT_LIMIT 8
#define RECONNECT_INTERVAL 5000 /*ms*/
// char *test_ssid = "QCA9531_18c0";
char *test_ssid = "YE_HCW10_ABCD";
// char *test_ssid = "Xiaomi_5AFD_5G";
char *test_password = "12345678";
u32 secure_type = RTW_SECURITY_WPA2_AES_PSK; /*Just distinguish between WEP, TKIP/WPA/WPA2/WPA3 can use the same secure_type*/
/***********************************End**********************************/
static const char *TAG = "WIFI_RECONN_EXAMPLE";
// extern int ble_central_main(uint8_t enable);
extern int ble_peripheral_main(uint8_t enable);

int user_wifi_connect(void)
{
#if 1
	return RTW_SUCCESS;
#endif
	int ret = 0;
	struct _rtw_network_info_t connect_param = {0};

	/*Connect parameter set*/
	memcpy(connect_param.ssid.val, test_ssid, strlen(test_ssid));
	connect_param.ssid.len = strlen(test_ssid);
	connect_param.password = (unsigned char *)test_password;
	connect_param.password_len = strlen(test_password);
	connect_param.security_type = secure_type;

	while (1)
	{
		/*Connect*/
		ret = wifi_connect(&connect_param, 1);
		if (ret != RTW_SUCCESS)
		{
			RTK_LOGI(TAG, "Reconnect Fail:%d", ret);
			if ((ret == RTW_CONNECT_INVALID_KEY))
			{
				RTK_LOGI(TAG, "(password format wrong)\r\n");
			}
			else if (ret == RTW_CONNECT_SCAN_FAIL)
			{
				RTK_LOGI(TAG, "(not found AP)\r\n");
			}
			else if (ret == RTW_BUSY)
			{
				RTK_LOGI(TAG, "(busy)\r\n");
			}
			else
			{
				RTK_LOGI(TAG, "(other)\r\n");
			}
		}

		/*DHCP*/
		if (ret == RTW_SUCCESS)
		{
			RTK_LOGI(TAG, "Wifi connect success, Start DHCP\n");
			ret = LwIP_DHCP(0, DHCP_START);
			if (ret == DHCP_ADDRESS_ASSIGNED)
			{
				RTK_LOGI(TAG, "DHCP Success\n");
				return RTW_SUCCESS;
			}
			else
			{
				RTK_LOGI(TAG, "DHCP Fail\n");
				wifi_disconnect();
			}
		}

		rtos_time_delay_ms(RECONNECT_INTERVAL);
	}
}

extern void wifi_fast_connect_enable(unsigned char enable);

static void user_main_task(void *param)
{
	(void)param;
	RTK_LOGI(TAG, "start\n");

	/* Wait wifi init finish */
	while (!(wifi_is_running(STA_WLAN_INDEX)))
	{
		rtos_time_delay_ms(1000);
	}
	wifi_fast_connect_enable(0);
	/* enable realtek auto reconnect */
	wifi_config_autoreconnect(0);

	my_ble_mode_t mode;
	const my_ble_config_t config = {320, 320, 0, 1000};
	my_ble_init(mode, &config);
	my_ble_set_name("hello_bt");
	uint8_t adv_data[] = {
		0x02,																				 // AD len
		RTK_BT_LE_GAP_ADTYPE_FLAGS,															 // AD types
		RTK_BT_LE_GAP_ADTYPE_FLAGS_GENERAL | RTK_BT_LE_GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED, // AD data
		0x12,
		RTK_BT_LE_GAP_ADTYPE_LOCAL_NAME_COMPLETE,
		'R', 'T', 'K', '_', 'B', 'T', '_', 'h', 'E', 'R', 'I', 'P', 'H', 'E', 'R', 'A', 'L',
		0x04,
		0xee,
		0x01, 0x02, 0x03};
	char sdData[] = {'h', 'e', 'l', 'l', 'o', 's', 'd'};
	my_ble_adv_start((uint8_t *)adv_data, sizeof(adv_data), (uint8_t *)sdData, sizeof(sdData));
	while (1)
	{
		const my_ble_scan_param_t cfg = {
			.type = RTK_BT_LE_SCAN_TYPE_ACTIVE,
			.interval = 1000,
			.window = 1000,
			.own_addr_type = RTK_BT_LE_ADDR_TYPE_RANDOM,
			.filter_policy = RTK_BT_LE_SCAN_FILTER_ALLOW_ALL,
			.duplicate_opt = 1,
		};
		my_ble_scan(&cfg);
		while (1)
		{
			rtos_time_delay_ms(500);
			// my_ble_scan_stop();
			// rtos_time_delay_ms(50);
		}
	}

	/* Start connect */
	if (RTW_SUCCESS == user_wifi_connect())
	{
		printf("creat skt\r\n");
		// ble_central_main(1);

		int sockfd;
		struct sockaddr_in broadcast_addr;
		char buffer[1024];
		int broadcast = 1;

		// 创建UDP socket
		sockfd = socket(AF_INET, SOCK_DGRAM, 0);
		if (sockfd < 0)
		{
			RTK_LOGI(TAG, "创建socket失败\n");
			return;
		}

		// 设置socket选项以允许广播
		if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)) < 0)
		{
			RTK_LOGI(TAG, "设置socket选项失败\n");
			close(sockfd);
			return;
		}

		// 配置广播地址
		memset(&broadcast_addr, 0, sizeof(broadcast_addr));
		broadcast_addr.sin_family = AF_INET;
		broadcast_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		broadcast_addr.sin_port = htons(8879);

		// 绑定socket到广播地址
		if (bind(sockfd, (struct sockaddr *)&broadcast_addr, sizeof(broadcast_addr)) < 0)
		{
			RTK_LOGI(TAG, "绑定socket失败\n");
			close(sockfd);
			return;
		}

		// 接收广播信息并发送到目标地址
		while (1)
		{
			struct sockaddr_in client_addr;
			socklen_t addr_len = sizeof(client_addr);
			int recv_len = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_addr, &addr_len);
			if (recv_len > 0)
			{
				printf("client_addr: %s\r\n", inet_ntoa(client_addr.sin_addr));
				for (int i = 0; i < recv_len; i++)
				{
					printf("%02X ", buffer[i]);
				}
				printf("\r\n");

				// sendto(sockfd, buffer, recv_len, 0, (struct sockaddr *)&client_addr, addr_len);
			}
		}

		close(sockfd);

		printf("rtos_task_delete\r\n");
	}

	while (!((wifi_get_join_status() == RTW_JOINSTATUS_SUCCESS) && (*(u32 *)LwIP_GetIP(0) != IP_ADDR_INVALID)))
	{
		RTK_LOGS(NOTAG, "Wait for WIFI connection ...\n");
		rtos_time_delay_ms(2000);
	}

	rtos_task_delete(NULL);
}

void app_main(void)
{
	// printf("Hello World\n");
	RTK_LOGI(TAG, "startaaa\n");

	if (rtos_task_create(NULL, ((const char *)"user_main_task"), user_main_task, NULL, 2048, 1) != SUCCESS)
	{
		RTK_LOGI(TAG, "\n%s rtos_task_create failed\n", __FUNCTION__);
	}
}