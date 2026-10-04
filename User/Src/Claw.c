#include "Claw.h"

// 变量定义
Claw claw = {0};
Claw_Height height = {0};
Claw_Deg deg = {0};

static float Current_mm = REF_MM;
static float Target_deg_1 = 0.0f;
static float Target_deg_2 = 0.0f;

void Claw_Init(void)
{
    claw.Begin = true;
    claw.MODE_Set = Mode_Zero;
    claw.MODE_Cur = Mode_Zero;
    claw.MotivateFlag = false;
    claw.SoleMode = 0; /* 0=全关,见 Claw_Sole() */

    height.GetEarth = 275.0f;
    height.PlaceEarth = 280.0f;
    height.HoldEarth = 290.0f;
    height.Ready = 354.0f;
    
    Current_mm = REF_MM;
    Target_deg_1 = 0.0f;
    Target_deg_2 = 0.0f;

    deg.Ready = 180.0f;
    deg.Test = 270.0f;
}

/**
 * @brief 把目标高度(mm)换算成两个电机的目标角度,并按相对位移下发。
 * @note  使用 Zmotor[] 的 pos_deg 反馈作增量基准。
 */
void Change_to_Height(float Target_mm)
{
    float Relative_mm = Target_mm - Current_mm;
    float Relative_deg = (Relative_mm / PA_MM_PER_REV) * 360.0f;

    /* 相对当前位置(机构装好后实测这里该用加号还是减号) */
    Target_deg_1 = Zmotor[CLAW_MOTOR_1].valReal.pos_deg - Relative_deg;
    Zmotor[CLAW_MOTOR_1].valSetNow.pos_deg = Target_deg_1;

    Target_deg_2 = Zmotor[CLAW_MOTOR_2].valReal.pos_deg - Relative_deg;
    Zmotor[CLAW_MOTOR_2].valSetNow.pos_deg = Target_deg_2;

    while (ABS(Zmotor[CLAW_MOTOR_1].valSetNow.pos_deg - Zmotor[CLAW_MOTOR_1].valReal.pos_deg) > 5U) // 约2mm误差
    {
        osDelay(1);
    }

    Current_mm = Target_mm;
}

void Change_to_Deg(float Target_deg)
{
    Zmotor[MOTOR_RS03_NUM].valSetNow.pos_deg = Target_deg;

    while (ABS(Zmotor[MOTOR_RS03_NUM].valSetNow.pos_deg - Zmotor[MOTOR_RS03_NUM].valReal.pos_deg) > 0.5f)
    {
        osDelay(50);
    }
}

void Claw_Motivate()
{
    if (claw.MotivateFlag == false)
    {
        claw.Begin = false;

        for (uint32_t i = 0; i < MOTOR_ZDRIVE_COUNT; i++)
        {
            Zmotor[i].Begin = false;
            Zmotor[i].mode = Zdrive_Disable;
        }

        solenoid_on(3, 0);

        Beep_Alarm(2);
    }
    else if (claw.MotivateFlag == true)
    {
        for (uint32_t i = 0; i < MOTOR_ZDRIVE_COUNT; i++)
        {
            Zmotor[i].Begin = true;
            Zmotor[i].mode = Zdrive_Postion;
        }

        claw.Begin = true;

        Beep_Alarm(1);
    }
}

/*先按用1,2号口来写，测的时候再根据具体情况改*/
void Claw_Sole()
{
    switch (claw.SoleMode)
    {
    case 0: // 全关
        solenoid_on(3, 0);
        break;

    case 1: // 全开
        solenoid_on(3, 15);
        break;

    case 2: // 1号开2号关
        solenoid_on(3, 1);
        break;
    case 3: // 1号关2号开
        solenoid_on(3, 2);
        break;

    default:
        break;
    }
}

void Claw_Ready()
{
    Change_to_Height(height.Ready);
    osDelay(100);

    Change_to_Deg(deg.Ready);
    osDelay(50);

    solenoid_on(3, 15);
}

void Claw_Rs03()
{
    solenoid_on(3, 0);

    Change_to_Deg(deg.Ready);
    osDelay(50);

}

void Claw_GetEarth()
{
    Change_to_Height(height.GetEarth);
    osDelay(100);

    solenoid_on(3, 0);
}

void Claw_PlaceEarth()
{
    Change_to_Height(height.PlaceEarth);
    osDelay(100);

    solenoid_on(3, 15);
}

void Claw_Zero()
{
    solenoid_on(3, 15);

    Change_to_Height(height.Ready);
    osDelay(100);

    solenoid_on(3, 0); // 这里要想想怎么保证合爪时一定不会夹着东西

    Change_to_Deg(0.0f);
    osDelay(50);

    Change_to_Height(REF_MM);
}

void Claw_Error()
{
    /* TODO: 错误处理 */
}

void Claw_Reset()
{
    for (uint32_t i = 0; i < MOTOR_AK60_COUNT; i++)
    {
        Zmotor[i].Begin = false;
        Zmotor[i].mode = Zdrive_Disable;
    }

    BEEP_ON();
    osDelay(100);
    BEEP_OFF();
    osDelay(20);

    __set_FAULTMASK(1);
    NVIC_SystemReset();
    osDelay(1);
}

void Claw_Func(void)
{
    if (!claw.Begin)
    {
        return;
    }

    /* MODE_Set == 0 表示无新指令,直接返回 */
    if (claw.MODE_Set == 0)
    {
        return;
    }

    claw.MODE_Cur = claw.MODE_Set;
    claw.MODE_Set = 0;

    switch (claw.MODE_Cur)
    {
    case Mode_Motivate:
        Claw_Motivate(); /* TODO: 夹爪电机使能 */
        break;

    case Mode_Sole:
        Claw_Sole(); /* TODO: 夹爪开合 */
        break;

    case Mode_RS03:
        Claw_Rs03(); /* TODO: 夹爪旋转测试用 */
        break;

    case Mode_Ready:
        Claw_Ready();
        break;

    case Mode_GetEarth:
        Claw_GetEarth();
        break;

    case Mode_PlaceEarth:
        Claw_PlaceEarth();
        break;

    case Mode_Zero:
        Claw_Zero();
        break;

    case Mode_Error:
        Claw_Error(); /* TODO: 错误处理 */
        break;

    case Mode_Reset:
        Claw_Reset();
        break;

    default:
        break;
    }
}