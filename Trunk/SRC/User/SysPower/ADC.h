/***************************Copyright BestFu ***********************************
**  文    件：  ADC.h
**  功    能：  <<驱动层>> ADC驱动 
**  编    译：  Keil uVision5 V5.10
**  版    本：  V1.0
**  编    写：  Seven
**  创建日期：  2014.08.20
**  修改日期：  2014.08.20
**  说    明：  
**  V1.0
    >> STM32L151C8T6  ADC1 驱动
*******************************************************************************/

#ifndef _ADC_H_
#define _ADC_H_

#include "sys.h"
#include "GPIO.h"

//定义ADC采样通道
#define PORT_ADC            GPIO_A
#define PIN_ADC             pin_4
#define ADC_CH              4
//ADC采样电路电源开关
#define PORT_ADC_SWITCH     GPIO_A
#define PIN_ADC_SWITCH      pin_5
                                                   
extern void ADC_Init(void);                             //初始化ADC
extern void ADC_TestPowerON(void);                      //ADC检测电路 电源开
extern void ADC_TestPowerOFF(void);                     //ADC检测电路 电源关
extern u8   ADC_Read(u16 *adcResult);                   //读取采样值
extern u8   ADC_ReadAverage(u8 times,u16 *adcAverage);  //读取采样的平均值 

#endif 

/***************************Copyright BestFu **********************************/
