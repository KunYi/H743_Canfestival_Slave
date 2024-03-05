/**
  ******************************************************************************
  * File Name          : bsp_canopen_timer.h
  * Description        :
  *
  * Designer		   : song.wang@ltsmart.com.cn
  *
  ******************************************************************************
  * @attention
  *
  * 用于为canfestival提供硬件接口，定时器部分
  * tim.c为cubeMX生成 该文件搭建 硬件(tim)与canfestival的桥梁
  *
  ******************************************************************************
  */


#ifndef __BSP_CANOPEN_TIMER_H
#define __BSP_CANOPEN_TIMER_H


#ifdef __cplusplus
   extern "C" {
#endif
  /***********************************************/
#include "main.h"
#include "canfestival.h"
#include "tim.h"

//该文件的接口配置 根据cubeMX设置进行修改
#define CANOPEN_TIMx              (TIM_HandleTypeDef *)&htim17
#define CANOPEN_TIM_PERIOD        65535          //定时周期 与cubeMX中设置相同

//void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);
void TIMx_DispatchFromISR(void);

/***********************************************/
#ifdef __cplusplus
}
#endif

#endif /*__BSP_CANOPEN_TIMER_H */



