#ifndef __SOLENOID_H
#define __SOLENOID_H

#include "main.h"
#include "can.h"

/* ------------------------------------------------------------------ */
/* 电磁阀板 CAN 控制协议(与电磁阀板固件一致)                          */
/* ------------------------------------------------------------------ */

/** @brief 电磁阀板接收的控制帧 CAN 标准 ID(与电磁阀板固件 TARGET_ID 一致) */
#define SOLENOID_CAN_ID 0x123U

/** @brief 使用的 CAN 总线:0 = CAN1,1 = CAN2(需与电磁阀板挂在同一条总线上) */
#define SOLENOID_CAN_BUS 0U

/**
 * @brief 初始化电磁阀控制模块。
 * @note  须在 MX_CANx_Init() 之后调用。CAN 的启动与过滤器仍由
 *        Core/Src/can.c 管理,本模块只负责发送,不重复配置。
 */
void solenoid_init(void);

/**
 * @brief 按位图设置 4 路电磁阀输出,并通过 CAN 下发到电磁阀板。
 * @param usart_channel 历史遗留参数(保留以兼容 Claw.c 调用),当前不参与报文组装
 * @param cmd 控制字节,仅低 4 位有效:
 *            bit0 -> CH1,bit1 -> CH2,bit2 -> CH3,bit3 -> CH4
 *            1 = 开启,0 = 关闭
 * @note  每次调用都会下发一帧 CAN(ID=SOLENOID_CAN_ID, DLC=1)。
 */
void solenoid_on(uint8_t usart_channel, uint8_t cmd);

/**
 * @brief 读回最后一次下发的控制字节(低 4 位有效)。主要用于调试。
 */
uint8_t solenoid_get_state(void);

#endif /* __SOLENOID_H */
