/**
  ******************************************************************************
  * @file    wifi_init.c
  * @author
  * @version
  * @date
  * @brief
  ******************************************************************************
  * @attention
  *
  * This module is a confidential and proprietary property of RealTek and
  * possession or use of this module requires written permission of RealTek.
  *
  * Copyright(c) 2024, Realtek Semiconductor Corporation. All rights reserved.
  ******************************************************************************
  */
#include "ameba_soc.h"
#ifdef CONFIG_WLAN
#if defined(CONFIG_AS_INIC_AP) && defined(CONFIG_SPI_FULLMAC_HOST)  && CONFIG_SPI_FULLMAC_HOST
#include "inic_spi_host.h"
#else
#include "inic_ipc.h"
#endif
#include "wifi_conf.h"
#if defined(CONFIG_LWIP_LAYER) && CONFIG_LWIP_LAYER
#include "lwip_netconf.h"
#endif
#ifndef CONFIG_AS_INIC_NP
#include "wifi_fast_connect.h"
#if defined(CONFIG_AS_INIC_AP) || (defined(CONFIG_LWIP_LAYER) && CONFIG_LWIP_LAYER && defined(CONFIG_SINGLE_CORE_WIFI))
static u32 heap_tmp;
#endif
#endif

#define WIFI_STACK_SIZE_INIT ((512 + 768) * 4)
__attribute__((unused)) static const char *TAG = "WLAN";
extern void wifi_set_rom2flash(void);

#ifndef CONFIG_AS_INIC_AP
void init_skb_pool(uint32_t skb_num_np, unsigned char skb_cache_zise)
{
	int i;
	uint32_t skb_buf_size = wifi_user_config.skb_buf_size ? wifi_user_config.skb_buf_size : MAX_SKB_BUF_SIZE;

	if (skbpriv.skb_buff_pool) {
		RTK_LOGW(TAG_WLAN_DRV, "skb_buff_pool not mfree|\n");
		return;
	}

	skbpriv.skb_buf_max_size = ((skb_buf_size + (skb_cache_zise - 1)) & ~(skb_cache_zise - 1));
	skbpriv.skb_buff_num = skb_num_np;
	skbpriv.skb_buff_used = 0;
	skbpriv.skb_buff_max_used = 0;

	/*Start address must align to max(AP_Core_Cache, NP_Core_Cache),
	* freertos's portBYTE_ALIGNMENT usually guarantees this,
	* or we have to do align opreration here */
	skbpriv.skb_buff_pool = (struct sk_buff *)rtos_mem_zmalloc(skbpriv.skb_buff_num * sizeof(struct sk_buff));
	if (!skbpriv.skb_buff_pool) {
		RTK_LOGE(TAG_WLAN_DRV, "skb pool malloc fail\n");
	}

	INIT_LIST_HEAD(&skbpriv.skb_buff_list);
	for (i = 0; i < skbpriv.skb_buff_num; i++) {
		INIT_LIST_HEAD(&skbpriv.skb_buff_pool[i].list);
		list_add_tail(&skbpriv.skb_buff_pool[i].list, &skbpriv.skb_buff_list);
	}
}
#endif

#if defined(CONFIG_AS_INIC_AP)
void _init_thread(void *param)
{
	/* To avoid gcc warnings */
	(void) param;
#ifndef CONFIG_SPI_FULLMAC_HOST
	u32 val32 = 0;
#endif

#ifdef CONFIG_LWIP_LAYER
	/* Initilaize the LwIP stack */
	LwIP_Init();
#endif

#ifndef CONFIG_SPI_FULLMAC_HOST
	/* wait for inic_ipc_device ready, after that send WIFI_ON ipc msg to device */
	while ((HAL_READ32(REG_AON_WIFI_IPC, 0) & AON_BIT_WIFI_INIC_NP_READY) == 0) {
		rtos_time_delay_ms(1);
	}
	val32 = HAL_READ32(REG_AON_WIFI_IPC, 0);
	val32 &= ~ AON_BIT_WIFI_INIC_NP_READY;
	HAL_WRITE32(REG_AON_WIFI_IPC, 0, val32);
#endif

	wifi_on(RTW_MODE_STA);

#if CONFIG_AUTO_RECONNECT
	//setup reconnection flag
	wifi_config_autoreconnect(1);
#endif
	heap_tmp = heap_tmp - rtos_mem_get_free_heap_size() - WIFI_STACK_SIZE_INIC_IPC_HST_API - WIFI_STACK_SIZE_INIC_MSG_Q - WIFI_STACK_SIZE_INIT;
#ifdef CONFIG_LWIP_LAYER
	heap_tmp -= TCPIP_THREAD_STACKSIZE * 4;
#endif
	RTK_LOGI(TAG, "AP consume heap %d\n", heap_tmp);
	RTK_LOGI(TAG, "%s(%d), Available heap %d\n", __FUNCTION__, __LINE__, rtos_mem_get_free_heap_size() + WIFI_STACK_SIZE_INIT);

	/* Kill init thread after all init tasks done */
	rtos_task_delete(NULL);
}

void wlan_initialize(void)
{
	heap_tmp = rtos_mem_get_free_heap_size();
	wifi_set_rom2flash();
	inic_host_init();
	wifi_fast_connect_enable(1);

	if (rtos_task_create(NULL, ((const char *)"init"), _init_thread, NULL, WIFI_STACK_SIZE_INIT, 2) != SUCCESS) {
		RTK_LOGE(TAG, "wlan_initialize failed\n");
	}
}

#elif defined(CONFIG_AS_INIC_NP)
void wlan_initialize(void)
{
	u32 value;
	wifi_set_rom2flash();
	inic_dev_init();

	/* set AON_BIT_WIFI_INIC_NP_READY=1 to indicate inic_ipc_device is ready */
	value = HAL_READ32(REG_AON_WIFI_IPC, 0);
	HAL_WRITE32(REG_AON_WIFI_IPC, 0, value | AON_BIT_WIFI_INIC_NP_READY);
}

#elif defined(CONFIG_SINGLE_CORE_WIFI)
#include "wifi_fast_connect.h"
void _init_thread(void *param)
{
	/* To avoid gcc warnings */
	(void) param;

#if defined(CONFIG_ARM_CORE_CM4) && defined(configENABLE_TRUSTZONE) && (configENABLE_TRUSTZONE == 1)
	rtos_create_secure_context(configMINIMAL_SECURE_STACK_SIZE);
#endif

#ifdef CONFIG_LWIP_LAYER
	/* Initilaize the LwIP stack */
	heap_tmp = rtos_mem_get_free_heap_size();
	LwIP_Init();
	RTK_LOGI(TAG, "LWIP consume heap %d\n", heap_tmp - rtos_mem_get_free_heap_size() - TCPIP_THREAD_STACKSIZE * 4);
#endif
#ifdef CONFIG_SDIO_BRIDGE
	wifi_fast_connect_enable(0);
	inic_dev_init();
#endif
	wifi_set_user_config();

	wifi_on(RTW_MODE_STA);
#if CONFIG_AUTO_RECONNECT
	//setup reconnection flag
	wifi_config_autoreconnect(1);
#endif

	RTK_LOGI(TAG, "%s(%d), Available heap %d\n", __FUNCTION__, __LINE__, rtos_mem_get_free_heap_size() + WIFI_STACK_SIZE_INIT);

	/* Kill init thread after all init tasks done */
	rtos_task_delete(NULL);
}

void wlan_initialize(void)
{
	wifi_set_rom2flash();

	wifi_fast_connect_enable(1);

	if (rtos_task_create(NULL, ((const char *)"init"), _init_thread, NULL, WIFI_STACK_SIZE_INIT, 5) != SUCCESS) {
		RTK_LOGE(TAG, "wlan_initialize failed\n");
	}
}

#endif
#endif
