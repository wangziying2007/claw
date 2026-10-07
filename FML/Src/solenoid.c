/*
 * @Author: Frt001 2067314783@qq.com
 * @Date: 2026-08-24 16:51:06
 * @LastEditors: Frt001 2067314783@qq.com
 * @LastEditTime: 2026-10-07 00:00:00
 * @FilePath: \f4_show\FML\Src\solenoid.c
 * @Description: 电磁阀控制。通过 CAN(ID=0x123, DLC=1)把控制字节下发给电磁阀板,
 *               4 路输出由 Data[0] 的低 4 位控制,已取代本地 GPIO 软件移位方案。
 */
#include "solenoid.h"

/** @brief 本模块使用的 CAN 句柄(由 solenoid_init 绑定) */
static CAN_HandleTypeDef *s_solenoid_can = NULL;

/** @brief 最后一次成功下发的控制字节(低 4 位有效),仅用于调试读回 */
static uint8_t s_solenoid_state = 0x00U;

/**
 * @brief 取本模块使用的 CAN 总线句柄。
 */
static CAN_HandleTypeDef *Solenoid_GetCan(void)
{
#if (SOLENOID_CAN_BUS == 0U)
    return &hcan1;
#elif (SOLENOID_CAN_BUS == 1U)
    return &hcan2;
#else
    return NULL;
#endif
}

/**
 * @brief 初始化电磁阀控制模块(绑定 CAN 句柄)。
 * @note  须在 MX_CANx_Init() 之后调用。
 */
void solenoid_init(void)
{
    s_solenoid_can = Solenoid_GetCan();
    s_solenoid_state = 0x00U;
}

/**
 * @brief 按位图设置 4 路电磁阀输出,并通过 CAN 下发。
 * @param usart_channel 历史遗留参数(保留以兼容 Claw.c 的调用),不参与报文组装
 * @param cmd 控制字节,仅低 4 位有效:bit0->CH1, bit1->CH2, bit2->CH3, bit3->CH4, 1=开 0=关
 */
void solenoid_on(uint8_t usart_channel, uint8_t cmd)
{
    CAN_TxHeaderTypeDef tx_header;
    uint32_t tx_mailbox;
    uint8_t tx_data[1];
    uint8_t data = cmd & 0x0FU;

    (void)usart_channel; /* 报文内容只由 data 决定,通道参数仅用于兼容旧接口 */

    s_solenoid_state = data;

    if (s_solenoid_can == NULL)
        return;

    tx_data[0] = data;

    tx_header.StdId = SOLENOID_CAN_ID;
    tx_header.ExtId = 0U;
    tx_header.IDE = CAN_ID_STD;
    tx_header.RTR = CAN_RTR_DATA;
    tx_header.DLC = 1U;
    tx_header.TransmitGlobalTime = DISABLE;

    /* 每次调用都实际下发一帧:不做"值未变化则跳过"的去重,
       避免电磁阀板漏收一帧后相同的值永远发不出去。 */
    if (HAL_CAN_AddTxMessage(s_solenoid_can, &tx_header, tx_data, &tx_mailbox) != HAL_OK)
    {
        /* 发送失败(无空闲邮箱/总线异常):清空发送请求,避免占用邮箱影响后续发送 */
        (void)HAL_CAN_AbortTxRequest(s_solenoid_can, 0x07U);
    }
}

/**
 * @brief 读回最后一次下发的控制字节(低 4 位有效)。主要用于调试。
 */
uint8_t solenoid_get_state(void)
{
    return s_solenoid_state;
}
