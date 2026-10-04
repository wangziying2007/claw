/*
 * @Author: Frt001 2067314783@qq.com
 * @Date: 2026-08-11 10:06:00
 * @LastEditors: Frt001 2067314783@qq.com
 * @LastEditTime: 2026-08-25 16:53:33
 * @FilePath: \f4_show\IRQ\Src\TIM_IRQHandler.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "tim_irqhandler.h"
#include "motor_config.h"
#include "Claw.h"

/* 上电初始化延时:等 AK60 上电自检完成、CAN 反馈帧已能收到,再发置零/使能命令。
   时基来源(必须与 Core/Src/tim.c 的 MX_TIM2_Init 保持一致):
       TIM2 = 84MHz / (Prescaler+1) / (Period+1) = 84M/84/1000 = 1kHz
   即 1 个 tick = 1ms。改 TIM2 分频/周期时,只需改 CLAW_BOOT_MAIN_FREQ_HZ。 */
#define CLAW_BOOT_MAIN_FREQ_HZ 1000U /* TIM2 中断频率(Hz),见 MX_TIM2_Init */
#define CLAW_BOOT_DELAY_MS 200U      /* 期望的上电延时(ms) */
#define CLAW_BOOT_DELAY_TICKS ((CLAW_BOOT_DELAY_MS * CLAW_BOOT_MAIN_FREQ_HZ) / 1000U)


void TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if(htim->Instance == TIM2)
    {
        #if USE_ZMDR
            ZdriveDequeue((uint8_t)MOTOR_ZDRIVE_CAN_BUS_1);
            ZdriveDequeue((uint8_t)MOTOR_ZDRIVE_CAN_BUS_2);
        #endif        

        #if USE_DJ
            DJmotor_Func();
        #endif
        static uint8_t Func_cnt = 0;
        if (++Func_cnt >= 5)
        {
            Func_cnt = 0;
            #if USE_VESC
                VescFunc();
            #endif
            #if USE_ZMDR
                ZdriveFunc();
            #endif
        }
    }
    if(htim->Instance == TIM3){

    }

}
