/***************************Copyright BestFu ***********************************
**  文    件：  BatteryExeAttr.c
**  功    能：  <<属性层>> 电池属性接口 
**  编    译：  Keil uVision5 V5.10
**  版    本：  V1.1
**  编    写：  Seven
**  创建日期：  2014.08.20
**  修改日期：  2014.08.29
**  说    明：  
**  V1.1
    >> 取消读取电量值时每次测量  改为上电首次测量的电量值
*******************************************************************************/
#include "BatteryExeAttr.h"
#include "ADC.h"
#include "Battery.h"
#include "Thread.h"

u8 ADC_GetFlag;     // 1 获取成功   0 获取失败

//对属性读写的接口列表
//const AttrExe_st BatteryAttrTable[] =
//{
//    {0xC8, LEVEL_1, NULL    , Get_PowerPercent_Attr },   //固定属性  电池电量获取 
//    {0xC9, LEVEL_1, NULL    , Get_ChargeState_Attr  },   //固定属性  电池充电状态 
//};

/*******************************************************************************
**函    数： Battery_Init
**功    能： 电池属性初始化
**参    数： void  
**返    回： void
*******************************************************************************/
void Battery_Init(void)
{
    ADC_Init();
    if(1 == Battery_GetPercent(&gPowerPercent))
    {
        ADC_GetFlag = 1;    //成功
    }
    else ADC_GetFlag = 0;   //失败
}

/*******************************************************************************
**函    数： Get_PowerPercent_Attr
**功    能： 获取电池电量百分比
**参    数： *pData        -- 输入参数指针
**           *rlen         -- 返回参数长度
**           *rData        -- 返回参数指针
**返    回： COMPLETE：成功    
*******************************************************************************/
MsgResult_t Get_PowerPercent_Attr(UnitPara_t *pData, u8 *rLen, u8 *rData)
{
    //if (1 == Battery_GetPercent(&gPowerPercent))
    if (1 == ADC_GetFlag)
    {
        *rData = gPowerPercent;
        *rLen = 1;   
        return COMPLETE;        
    }
    else
    {
        return CMD_EXE_ERR;
    }
}

/*******************************************************************************
**函    数： Get_ChargeState_Attr
**功    能： 获取充电状态
**参    数： *pData        -- 输入参数指针
**           *rlen         -- 返回参数长度
**           *rData        -- 返回参数指针
**返    回： COMPLETE：成功    
**说    明： (此功能暂未实现)
*******************************************************************************/
MsgResult_t Get_ChargeState_Attr(UnitPara_t *pData, u8 *rlen, u8 *rpara)
{
    return COMPLETE;
}

/***************************Copyright BestFu **********************************/
