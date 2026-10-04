#ifndef CLAW_H
#define CLAW_H

#include <stdbool.h>
#include "beep.h"
#include "led.h"
#include "solenoid.h"
#include "zdrive.h"
#include "main.h"
#include "can.h"
#include "cmsis_os2.h"

#define REF_MM 0.0f           /* 上电起点中间横杆的绝对参考高度(mm) */
#define PA_MM_PER_REV 157.08f /* 电机输出轴转 1 圈 = 157.08mm */

/* 夹爪机构使用的两个 AK60 电机在 Zmotor[] 中的索引 */
#define CLAW_MOTOR_1 0U
#define CLAW_MOTOR_2 1U

#define ABS(x) ((x) > 0 ? (x) : (-(x)))

typedef struct
{
    float Ready;
    float GetEarth;
    float PlaceEarth;
    float HoldEarth;
} Claw_Height;

/* RS03 旋转轴的目标角度(deg) */
typedef struct
{
    float Ready; /* 预备姿态角度 */
    float Test;  /* 测试姿态角度 */
} Claw_Deg;

typedef enum
{
    Mode_Motivate = 1,
    Mode_Sole,
    Mode_RS03,
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
    volatile bool MotivateFlag;
    volatile uint8_t SoleMode;
    volatile Claw_mode MODE_Set;
    volatile Claw_mode MODE_Cur;
} Claw, *ClawPointer;

extern Claw claw;
extern Claw_Height height;
extern Claw_Deg deg;

void Claw_Init(void);
void Claw_Func(void);

/* ---------------- mode 封装函数 ---------------- */
/* 每个 Claw_mode 对应一个动作函数,由 Claw_Func 的 switch 分派调用 */

/** @brief Mode_Motivate:夹爪电机上使能并切到位置模式 */
void Claw_Motivate(void);

/** @brief Mode_Sole:单边动作(TODO 待实现) */
void Claw_Sole(void);

/** @brief Mode_RS03:RS03 旋转轴动作(测试用) */
void Claw_Rs03(void);

/** @brief Mode_Ready:运动到预备高度 height.Ready */
void Claw_Ready(void);

/** @brief Mode_GetEarth:运动到取土高度 height.GetEarth */
void Claw_GetEarth(void);

/** @brief Mode_PlaceEarth:运动到放土高度 height.PlaceEarth */
void Claw_PlaceEarth(void);

/** @brief Mode_Zero:回到绝对参考高度 REF_MM */
void Claw_Zero(void);

/** @brief Mode_Error:错误处理(TODO 待实现) */
void Claw_Error(void);

/** @brief Mode_Reset:复位夹爪状态,内部调用 Claw_Init() */
void Claw_Reset(void);

/**
 * @brief 把目标高度(mm)换算成两个电机的目标角度,并按相对位移下发。
 * @param Target_mm 目标高度,单位 mm
 * @note  内部维护 Current_mm 作为"上一目标高度",实现增量式运动。
 */
void Change_to_Height(float Target_mm);

/**
 * @brief 把目标角度(deg)下发给 RS03 旋转轴,并等待到位。
 * @param Target_deg 目标角度,单位 deg
 * @note  使用 Zmotor[MOTOR_RS03_NUM] 的 pos_deg 反馈判断到位。
 */
void Change_to_Deg(float Target_deg);

#endif