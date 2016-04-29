/***************************Copyright BestFu 2016-01-28*************************
文 件：    TimeHandle.c
说 明：    时间处理函数
编 译：    Keil uVision V5.12.0.0
版 本：    v1.0
编 写：    jay
日 期：    2016-03-09
修  改:    无
*******************************************************************************/
#include "TimeHandle.h"

#if (DALAY_EXECUTE_EN > 0u)
//variable define

//function declaration
static u8 CheckTimePara(u16 time);
static u8 CheckTimeParaIsOK(u16 delaytime,u16 keeptime,u8 repeat);

/*******************************************************************************
函 数 名：  u32 TransformTimePara(u16* time)
功能说明：  将延时保持时间转换成秒
参   数：   time :延时保持时间
返 回 值：  转换成秒的时间
说    明：  无
*******************************************************************************/
u32 TransformTimePara(u16* time)
{
	u32 return_result = 0u;
	TimeAnalyze_t *pTimeAnalyze = (TimeAnalyze_t*)(time);
	
	switch(pTimeAnalyze->SecAnalyze.flag)
	{
		case 0:
		{
			return_result = (pTimeAnalyze->SecAnalyze.second) * 10 +\
							(pTimeAnalyze->SecAnalyze.minute * 600) +\
							(pTimeAnalyze->SecAnalyze.hour * 36000);
		}break;
		case 1:
		{
			return_result = (pTimeAnalyze->MinAnalyze.minute * 600) +\
							(pTimeAnalyze->MinAnalyze.hour * 36000) +\
							(pTimeAnalyze->MinAnalyze.day * 864000);
		}break;
		case 2:
		{
			return_result = (pTimeAnalyze->HourAnalyze.hour * 36000) +\
							(pTimeAnalyze->HourAnalyze.day * 864000);
		}break;	
		case 3:
		{
			return_result = (pTimeAnalyze->MsecAnalyze.msec) +\
							(pTimeAnalyze->MsecAnalyze.second * 10) + 
							(pTimeAnalyze->MsecAnalyze.minute * 600);
		}
		default:
			break;
	}
	return (return_result);
}

/*******************************************************************************
函 数 名：  static u8 CheckTimePara(u16 time)
功能说明：  检查时间参数合法性
参   数：   time	: 被检查的时间参数
返 回 值：  检查结果(TRUE:表示时间参数正确,FALSE：表示时间参数不合法)
*******************************************************************************/
static u8 CheckTimePara(u16 time)
{
	TimeAnalyze_t* pTimeAnalyze = (TimeAnalyze_t *)(&time);
	switch(((TimeAnalyze_t *)(&time))->SecAnalyze.flag)
	{
		case 0:
		{
			if((pTimeAnalyze->SecAnalyze.second > 59)||\
				(pTimeAnalyze->SecAnalyze.minute > 59))
			{
				return FALSE;
			}
		}break;
		case 1:
		{
			if((pTimeAnalyze->MinAnalyze.hour > 23)||\
				(pTimeAnalyze->MinAnalyze.minute > 59))
			{
				return FALSE;
			}
		}break;
		case 2:
		{
			if((pTimeAnalyze->HourAnalyze.hour > 23))
			{
				return FALSE;
			}
		}break;
		case 3:
		{
			if((pTimeAnalyze->MsecAnalyze.msec > 9)||\
				(pTimeAnalyze->MsecAnalyze.second > 59))
			{
				return FALSE;
			}
		}break;
		default:
			return FALSE;
	}
	return TRUE;
}

/*******************************************************************************
函 数 名：  u16 CheckTimeParaIsZero(u16 timepara)
功能说明：  检查全部时间参数是否为零
参   数：   timepara	: 被检查的时间参数
返 回 值：  检查结果(若时间内参数为零则返回为零)
*******************************************************************************/
u16 CheckTimeParaIsZero(u16 timepara)
{
	return ((timepara & 0x3fff) > 0);
}

/*******************************************************************************
函 数 名：  static u8 CheckTimeParaIsOK(u16 delaytime,u16 keeptime,u8 repeat)
功能说明：  检查全部时间参数合法性
参   数：   delaytime	: 被检查的延时时间参数
			keeptime	: 被检查的保持时间参数
			repeat		: 被检查的重复次数参数
返 回 值：  检查结果(TRUE:表示时间参数正确,FALSE：表示时间参数不合法)
*******************************************************************************/
static u8 CheckTimeParaIsOK(u16 delaytime,u16 keeptime,u8 repeat)
{
	if((0 == CheckTimeParaIsZero(delaytime))&&\
		((CheckTimeParaIsZero(keeptime) > 0)||(repeat > 0))
	  )
	{
		return FALSE;
	}
	if(FALSE == CheckTimePara(delaytime))
	{
		return FALSE;
	}
	if(FALSE == CheckTimePara(keeptime))
	{
		return FALSE;
	}
	return TRUE;
}

/*******************************************************************************
函 数 名：  u8 CheckTimeParaIsOK(u16 delaytime,u16 keeptime,u8 repeat)
功能说明：  检查全部时间参数合法性
参   数：   time		: 被检查的延时保持时间参数指针
			cnt			: 被检查的参数的个数
返 回 值：  检查结果(TRUE:表示时间参数正确,FALSE：表示时间参数不合法)
*******************************************************************************/
u8 CheckAllTimeParaIsOK(DlyTimeData_t* time,u8 cnt)
{
	u8 i;
	for(i = 0;i < cnt;i ++)
	{
		if(FALSE == CheckTimeParaIsOK(*(u16*)time->delaytime,*(u16*)time->keeptime,*(u8*)&time->repeat))
		{
			return FALSE;
		}
		time ++;
	}
	return TRUE;
}


#endif
/******************************End of File************************************/
