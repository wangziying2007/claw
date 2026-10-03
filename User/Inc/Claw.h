#ifndef CLAW_H
#define CLAW_H

#include <stdbool.h>
#include "beep.h"
#include "led.h"
#include "solenoid.h"
#include "zdrive.h"
#include "main.h"
#include "can.h"

#define REF_MM 0.0f           /* 上电起点中间横杆的绝对参考高度(mm) */
#define PA_MM_PER_REV 157.08f /* 电机输出轴转 1 圈 = 157.08mm */

/* 夹爪机构使用的两个 AK60 电机在 Zmotor[] 中的索引 */
#define CLAW_MOTOR_1 0U
#define CLAW_MOTOR_2 1U

typedef struct
{
    float Ready;
    float GetEarth;
    float PlaceEarth;
} Claw_Height;

typedef enum
{
    Mode_Motivate = 1,
    Mode_Sole,
    Mode_Ready,
    Mode_GetEarth,
    Mode_PlaceEarth,
    Mode_Zero,
    Mode_Error,
    Mode_Reset,
} Claw_mode;

typedef struct
{
    volatile bool Begin;
    volatile Claw_mode MODE_Set;
    volatile Claw_mode MODE_Cur;
} Claw, *ClawPointer;

extern Claw claw;
extern Claw_Height height;

void Claw_Init(void);
void Claw_Func(void);

/**
 * @brief 把目标高度(mm)换算成两个电机的目标角度,并按相对位移下发。
 * @param Target_mm 目标高度,单位 mm
 * @note  内部维护 Current_mm 作为"上一目标高度",实现增量式运动。
 */
void Change_Height_to_Degree(float Target_mm);

#endif