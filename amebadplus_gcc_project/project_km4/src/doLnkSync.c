#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "string.h"
#include "driver/gpio.h"

#include <unistd.h>
#include <string.h>
#include <time.h>
/******************************************************************************
 * 常量定义
 */
#define     RS485_EN_PIN        12
static const char *TAG = "LNKAUTH";
/******************************************************************************
 * 变量类型定义
 */
typedef struct
{
	uint8_t head[4];
	uint8_t cmdId;
	uint8_t crcValue;
	union
	{
		uint8_t QuestNum;
		struct
		{
			uint8_t QuestNum;
			uint8_t usingLic[8];
			uint8_t leftTime[2]; 
		}QuestRsp;
		struct
		{
            uint8_t lic[8];
			uint8_t status;
		}SetLicRsp;
		struct
		{
			uint8_t lic[8];
		}SetLic;
	};
}frame_t;
/******************************************************************************
 * 变量声明
 */
extern uint8_t deviceId[8];
static void uart_txrx_task(void *arg);
/******************************************************************************
 * 变量定义
 */
static struct
{
	uint8_t rcvdRawCnt;				//接收到裸字节数量
	uint8_t frameSize;				//帧尺寸
	bool rxDone;	
	union
	{
		uint8_t rxBuf[sizeof(frame_t)];
		frame_t rxFrame;
	};
}lnkRx;
static struct
{
    uint8_t QuestNum;               //查询码
    uint8_t usingLic[8];            //使用中的授权码
    uint16_t leftTime;              //授权剩余时间

    bool hasSettingLic;             //标记是否有待设置的授权码
    uint8_t settingLic[8];          //待设置的授权码
}lnkStatus;
static struct
{
	frame_t txFrame;
    uint8_t txBuf[sizeof(frame_t)*2];
}lnkTx;
/******************************************************************************
 * 函数名称：crc8
 * 功能描述：计算crc8校验码
 * 输    入：buf, len
 * 输    出：无
 * 返    回：crc值
 */
static uint8_t crc8(uint8_t *buf,uint8_t len)
{
    uint8_t i,l,crc;
    uint16_t init=0;
    for(l=0;l<len;l++)
    {
        init^=(buf[l]*0x100);
        for(i=0;i<8;i++)
        {
            if(init&0x8000)
                init^=0x8380;
            init*=2;
        }
    }
    crc = init/0x100;
    return crc;
}
/******************************************************************************
 * 函数名称: lnkSync_init
 * 功能描述: 墙板接口同步功能初始化
 * 输    入: 无
 * 输    出: 无
 * 返    回: 无
 */
void lnkSync_init(void)
{
    do
    {
        gpio_config_t io_conf = {};
        //disable interrupt
        io_conf.intr_type = GPIO_INTR_DISABLE;
        //set as output mode
        io_conf.mode = GPIO_MODE_OUTPUT;
        //bit mask of the pins that you want to set
        io_conf.pin_bit_mask = 1 << RS485_EN_PIN;
        //disable pull-down mode
        io_conf.pull_down_en = 0;
        //disable pull-up mode
        io_conf.pull_up_en = 0;
        //configure GPIO with the given settings
        gpio_config(&io_conf);

        gpio_set_level(RS485_EN_PIN, 0);
    } while (0);
    
    static uart_config_t uart_config = {
        .baud_rate = 1200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    uart_driver_install(UART_NUM_1, 1024, 1024, 0, NULL, 0);
    uart_set_pin(UART_NUM_1, GPIO_NUM_5, GPIO_NUM_4, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_param_config(UART_NUM_1, &uart_config);

    xTaskCreate(uart_txrx_task, "uart_txrx_task", 8192*2, NULL, configMAX_PRIORITIES-2, NULL);

    memset(&lnkStatus, 0, sizeof(lnkStatus));
}
/******************************************************************************
 * 函数名称: lnkSync_getUsingLicLefttime
 * 功能描述: 获取正在使用的授权码和授权剩余时间
 * 输    入: 无
 * 输    出: lic
 * 返    回: 无
 */
uint16_t lnkSync_getUsingLicLefttime(uint8_t *lic)
{
    memcpy(lic, lnkStatus.usingLic, 8);
    return lnkStatus.leftTime;
}
/******************************************************************************
 * 函数名称: lnkSync_setNewLic
 * 功能描述: 设置新的授权码
 * 输    入: lic
 * 输    出: 无
 * 返    回: true/false
 */
bool lnkSync_setNewLic(uint8_t *lic)
{
    memcpy(lnkStatus.settingLic, lic, 8);
    lnkStatus.hasSettingLic = true;
    return true;
}
/******************************************************************************
 * 串口收发任务
 */
static uint8_t uart_sendQuest(void)
{
    frame_t *frame = &lnkTx.txFrame;
    uint8_t txSize = 7;

    frame->head[0] = 0xa1;
    frame->head[1] = 0xb3;
    frame->head[2] = 0xc5;
    frame->head[3] = 0xd7;

    frame->cmdId = 0x00;
    frame->QuestNum = lnkStatus.QuestNum;

    frame->crcValue = 0;
    frame->crcValue = crc8((uint8_t*)&lnkTx.txFrame, txSize);

    uint8_t *ptr = (uint8_t*)&lnkTx.txFrame;
    for(int i=0; i<txSize; i++)
    {
        lnkTx.txBuf[i * 2] = 0xe0 + (ptr[i] / 0x10);
        lnkTx.txBuf[i * 2 + 1] = 0xe0 + (ptr[i] % 0x10);
    }
    gpio_set_level(RS485_EN_PIN, 1);
    uart_flush_input(UART_NUM_1);
    uart_write_bytes(UART_NUM_1, lnkTx.txBuf, txSize*2);
    uart_wait_tx_done(UART_NUM_1, 500 / portTICK_RATE_MS);
    gpio_set_level(RS485_EN_PIN, 0);
    return frame->cmdId;
}
static uint8_t uart_sendSetLic(void)
{
    frame_t *frame = &lnkTx.txFrame;
    uint8_t txSize = 14;

    frame->head[0] = 0xa1;
    frame->head[1] = 0xb3;
    frame->head[2] = 0xc5;
    frame->head[3] = 0xd7;

    frame->cmdId = 0x03;
    memcpy(frame->SetLic.lic, lnkStatus.settingLic, 8);

    frame->crcValue = 0;
    frame->crcValue = crc8((uint8_t*)&lnkTx.txFrame, txSize);

    uint8_t *ptr = (uint8_t*)&lnkTx.txFrame;
    for(int i=0; i<txSize; i++)
    {
        lnkTx.txBuf[i * 2] = 0xe0 + (ptr[i] / 0x10);
        lnkTx.txBuf[i * 2 + 1] = 0xe0 + (ptr[i] % 0x10);
    }
    gpio_set_level(RS485_EN_PIN, 1);
    uart_flush_input(UART_NUM_1);
    uart_write_bytes(UART_NUM_1, lnkTx.txBuf, txSize*2);
    uart_wait_tx_done(UART_NUM_1, 500 / portTICK_RATE_MS);
    gpio_set_level(RS485_EN_PIN, 0);
    return frame->cmdId;
}
static bool uart_doRecvRsp(uint8_t sentCmdId)
{
    static uint8_t buf[256];    
    int rcvdCnt;
    lnkRx.rcvdRawCnt = 0;
    do
    {
        rcvdCnt = uart_read_bytes(UART_NUM_1, buf, sizeof(buf), 500 / portTICK_RATE_MS);
        for(int i=0; i<rcvdCnt; i++)
        {
            do
            {
                static uint8_t matched = 0;
                const char req[] = "Tell me who you are.";
                if(matched < (sizeof(req) - 1))
                {
                    if(req[matched] != buf[i])
                        matched = 0;
                    else
                        matched ++;
                    if(matched == (sizeof(req) - 1))
                    {
                        matched = 0;
                        static char str[32];
                        sprintf(str, "DEVICEID=%02X%02X%02X%02X%02X%02X%02X%02X", 
                            deviceId[0], deviceId[1], deviceId[2], deviceId[3],
                            deviceId[4], deviceId[5], deviceId[6], deviceId[7]);

                        gpio_set_level(RS485_EN_PIN, 1);
                        uart_flush_input(UART_NUM_1);
                        uart_write_bytes(UART_NUM_1, str, strlen(str));
                        uart_wait_tx_done(UART_NUM_1, 500 / portTICK_RATE_MS);
                        gpio_set_level(RS485_EN_PIN, 0);
                    }
                }
            } while (0);
            
            uint8_t byte = buf[i];
            uint8_t rxIndex = lnkRx.rcvdRawCnt / 2;
            if((byte & 0xf0) != 0xe0)
            {
                lnkRx.rcvdRawCnt = 0;
                continue;
            }
            lnkRx.rcvdRawCnt ++;
            if(lnkRx.rcvdRawCnt % 2 != 0)
                lnkRx.rxBuf[rxIndex] = (byte & 0x0f) * 0x10; 
            else
            {
                lnkRx.rxBuf[rxIndex] += (byte & 0x0f);
                
                if(rxIndex < 4)
                {
                    const uint8_t head[] = {0xa1, 0xb3, 0xc5, 0xd7};
                    lnkRx.frameSize = 0;
                    if(lnkRx.rxBuf[rxIndex] != head[rxIndex])
                    {
                        if(lnkRx.rxBuf[rxIndex] != 0xa1)
                        {
                            /* 数据移位 */
                            lnkRx.rxBuf[0] = lnkRx.rxBuf[rxIndex] * 0x10;
                            lnkRx.rcvdRawCnt = 1;
                        }
                        else
                        {
                            lnkRx.rxBuf[0] = lnkRx.rxBuf[rxIndex];
                            lnkRx.rcvdRawCnt = 2;
                        }
                    }
                }
                else if(rxIndex == 4)
                {
                    uint8_t cmdId = lnkRx.rxBuf[rxIndex];
                    /* 查询 */
                    if(sentCmdId == 0x00)
                    {
                        if(cmdId == 0x01)
                            lnkRx.frameSize = 7;    //无数据返回
                        else if(cmdId == 0x02)
                            lnkRx.frameSize = 17;   //有数据返回
                        else
                            lnkRx.rcvdRawCnt = 0;
                    }
                    else if(sentCmdId == 0x03)
                    {
                        if(cmdId == 0x04)
                            lnkRx.frameSize = 15;
                        else
                            lnkRx.rcvdRawCnt = 0;
                    }
                    else
                        lnkRx.rcvdRawCnt = 0;
                }
                else if(rxIndex == (lnkRx.frameSize - 1))
                {
                    lnkRx.rcvdRawCnt = 0;
                    uint8_t crcValue = lnkRx.rxFrame.crcValue;
                    lnkRx.rxFrame.crcValue = 0;
                    lnkRx.rxFrame.crcValue = crc8(lnkRx.rxBuf, lnkRx.frameSize);
                    if(crcValue != lnkRx.rxFrame.crcValue)
                        continue;
                    if(lnkRx.rxFrame.cmdId == 0x01)
                    {
                        ESP_LOGD(TAG, "Rcvd QUEST_Rsp without data");
                        return true;
                    }
                    else if(lnkRx.rxFrame.cmdId == 0x02)
                    {
                        lnkStatus.QuestNum = lnkRx.rxFrame.QuestRsp.QuestNum;
                        lnkStatus.leftTime = lnkRx.rxFrame.QuestRsp.leftTime[0] * 0x100;
                        lnkStatus.leftTime += lnkRx.rxFrame.QuestRsp.leftTime[1];
                        memcpy(lnkStatus.usingLic, lnkRx.rxFrame.QuestRsp.usingLic, 8);
                        ESP_LOGD(TAG, "Rcvd QUEST_Rsp with data[%02x%02x]", lnkRx.rxFrame.QuestRsp.leftTime[0], lnkRx.rxFrame.QuestRsp.leftTime[1]);
                        return true;
                    }
                    else if(lnkRx.rxFrame.cmdId == 0x04)
                    {
                        if(memcmp(lnkStatus.settingLic, lnkRx.rxFrame.SetLicRsp.lic, 8) == 0)
                        {
                            lnkStatus.hasSettingLic = false;
                            return true;
                        }
                    }
                }
            }	
        }
    } while (rcvdCnt > 0);
    return false;
}
static void uart_txrx_task(void *arg)
{
    while(1)
    {
        time_t sta, end;
        if(lnkStatus.hasSettingLic)
        {
            ESP_LOGD(TAG, "Send SET_LIC");
            uart_doRecvRsp(uart_sendSetLic());
            lnkStatus.QuestNum = 0;
            for(sta=time(NULL);;)
            {
                end = time(NULL);
                if(sta != end)
                    break;
                uart_doRecvRsp(0xff);
            }
            continue;
        }
        else 
        {
            ESP_LOGD(TAG, "Send QUEST");
            if(!uart_doRecvRsp(uart_sendQuest()))
            {
                /* 查询数据失败不能休息那么久 */
                for(sta=time(NULL);;)
                {
                    end = time(NULL);
                    if(sta != end)
                        break;
                    uart_doRecvRsp(0xff);
                }
                continue;
            }
        }
        for(int i=0; i<60; i++)
        {
            for(sta=time(NULL);;)
            {
                end = time(NULL);
                if(sta != end)
                    break;
                uart_doRecvRsp(0xff);
            }         
            /* 需要写授权的时候不要休息那么久 */
            if(lnkStatus.hasSettingLic)
                break;
        }
    }
}

