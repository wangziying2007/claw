#include "Claw.h";

// 变量定义
Claw claw;
Claw_Height height;

static float Current_mm = REF_MM;
static float Relative_mm = 0;
static float Target_deg_1 = 0;
static float Target_deg_2 = 0;

void Claw_Init(void)
{
    claw.Begin = true;
    claw.MODE_Set = Mode_Zero;
    claw.MODE_Cur = Mode_Zero;

    height.GetEarth = 300;
    height.PlaceEarth = 300;
    height.Ready = REF_MM;
}

static void Change_Height_to_Degree(float Target_mm)
{
    float Relative_mm = Target_mm - Current_mm;
    float Relative_deg = (Relative_mm / PA_MM_PER_REV) * 360.0f;
    Target_deg_1 = Zmotor[0].valReal.angle_deg - Relative_deg; // 相对当前位置（机构装好后实测来看这里是加号还是减号）
    Zmotor[0].valSet.angle_deg = Target_deg_1;

    Target_deg_2 = Zmotor[1].valReal.angle_deg - Relative_deg; // 相对当前位置（机构装好后实测来看这里是加号还是减号）
    Zmotor[1].valSet.angle_deg = Target_deg_2;

    Current_mm = Target_mm;
}

void Claw_Func(void)
{
    if (pump.Begin)
    {
        if (pump.MODE_Set = 0)
        {
            return;
        }

        pump.MODE_Cur = pump.MODE_Set;
        pump.MODE_Set = 0;

        switch (pump.MODE_Cur)
        {
        case Mode_Motivate:
            Pump_Motivate();
            break;

        case Mode_Sole:
            Pump_Sole();
            break;

        case Mode_Ready:
            Pump_Ready();
            break;

        case Mode_GetEarth:
            Pump_GetEarth();
            break;

        case Mode_PlaceEarth:
            Pump_PlaceEarth();
            break;

        case Mode_Zero:
            Pump_Zero();
            break;

        case Mode_Error:
            Pump_Error();
            break;

        case Mode_Reset:
            Pump_Reset();
            break;

        default:
            break;
        }
    }
}