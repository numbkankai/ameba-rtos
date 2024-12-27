#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#ifndef C_BW20_5G
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#else
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "Adapter.h"
#endif
/******************************************************************************
 * 常量定义
 */
#define GPIO_LED_G 16
#define GPIO_LED_Y 17

#define GPIO_LED_PIN_SEL ((1ULL << GPIO_LED_G) | (1ULL << GPIO_LED_Y))

#define SET_LED_G_ON() gpio_set_level(GPIO_LED_G, 0)
#define SET_LED_G_OFF() gpio_set_level(GPIO_LED_G, 1)
#define SET_LED_Y_ON() gpio_set_level(GPIO_LED_Y, 0)
#define SET_LED_Y_OFF() gpio_set_level(GPIO_LED_Y, 1)

/******************************************************************************
 * 函数名称: led_wifi_isConnected
 * 功能描述: 指示wifi是否连接
 * 输    入: true/false
 * 输    出: 无
 * 返    回: 无
 */
void led_wifi_isConnected(bool status)
{
    if (status)
        SET_LED_Y_ON();
    else
        SET_LED_Y_OFF();
}
/******************************************************************************
 * 函数名称: led_server_isActive
 * 功能描述: 指示连接服务器正常
 * 输    入: true/false
 * 输    出: 无
 * 返    回: 无
 */
void led_server_isActive(void)
{
    SET_LED_G_ON();
    usleep(100000);
    SET_LED_G_OFF();
}
/******************************************************************************
 * 函数名称: led_power_on
 * 功能描述: 指示灯上电显示
 * 输    入: 无
 * 输    出: 无
 * 返    回: 无
 */
void led_power_on(void)
{
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = GPIO_LED_PIN_SEL;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);

    usleep(500000);
    SET_LED_G_ON();
    SET_LED_Y_ON();
    usleep(500000);
    SET_LED_G_OFF();
    SET_LED_Y_OFF();
    sleep(1);
}
