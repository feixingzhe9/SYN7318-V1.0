/***************************Copyright BestFu 2014-05-14*************************
文	件：	Upload.c
说	明：	设备主动上报相关函数
编	译：	Keil uVision4 V4.54.0.0
版	本：	v1.0
编	写：	Unarty
日	期：	2014.07.03 
修　改：	暂无
*******************************************************************************/
#include "Upload.h"
#include "MsgPackage.h"
#include "UnitShare_Attr.h"
#include "Thread.h"
#include "SysTick.h"
#include "cmdProcess.h"
#include "FaultManage.h"

//Private Function declaration
static u8 Fault;
static UploadCondition_t UploadUnit[MAX_UPLOAD_UNIT];
static void Upload_Attr(u8 unit);
static void FaultMsg(void);

#ifndef   STM32L151  //常供电设备上报 yanhuan adding 2015/12/18
extern void SysTickUpload(void); 
static void Upload_Idle(void);
#endif 
/*******************************************************************************
函 数 名：	Upload_Fault
功能说明： 	主动异常上报
参	  数： 	upload: 异常情况具体见异常枚举歌词
返 回 值：	无
*******************************************************************************/
void Upload_Fault(Upload_t upload)
{
	u32 time;
	
	Fault = (u8)upload;
	if (upload == USER_DATA_RESET)
	{
		time = 1000;
	}
	else
	{
		time = 5;
	}
#ifndef   STM32L151 		//不是低功耗产品
	if(upload == RESTART)
		Thread_Login(ONCEDELAY, 0, 3000, &Upload_Idle);	//空闲时主动上报，防止无线死机
#endif 
	Thread_Login(ONCEDELAY, 0, time, &FaultMsg);
}
/*******************************************************************************
函 数 名：	FaultMsg
功能说明： 	异常消息封装
参	  数： 	upload: 异常情况具体见异常枚举歌词
返 回 值：	无
*******************************************************************************/
void FaultMsg(void)
{
	Msg_Upload(0, 0xFF, 1, (u8*)&Fault);
}
/*******************************************************************************
函 数 名：	Upload_Check
功能说明： 	设备属性状态上报时间核对
参	  数： 	无
返 回 值：	无
注    意:	无
*******************************************************************************/
void Upload_Check(void)
{
	u8 i = UnitCnt_Get();
	while(--i)
	{
		if(gUnitData[i].UploadAble)
		{		
			if(UploadUnit[i].flag == 1 && ((++UploadUnit[i].cnt)*20) >= UploadUnit[i].time)
			{
				Upload_Attr(i);	
				UploadUnit[i].flag =0;//不需上报，置0				
			}
		}
		else
			UploadUnit[i].flag =0;//不需上报，置0			
	}
    //检查是否还有需要上报的单元
	for(i = 1; i < UnitCnt_Get() ;i++) 
	{
		if(UploadUnit[i].flag == 1)//还有需要上报的单元
		{
			return ;
		}		
	}
	Thread_Logout(Upload_Check);
}
/*******************************************************************************
函 数 名：	Upload
功能说明： 	设备属性状态上报
参	  数： 	无
返 回 值：	无
注    意:	离散度10ms
*******************************************************************************/
void Upload(u8 unit)
{	
	u8 i,num ;
	if(unit)  
	{
		i = unit ;
		num = unit + 1 ;
	}
	else  //0 上报所有非0单元
	{
		i = 1 ;
		num = UnitCnt_Get();
	}
	for( ; i < num ; i++)
	{
		if(gUnitData[i].UploadAble)
		{
			UploadUnit[i].time = 2500 + (Time_Get()%gUnitData[i].UploadSetTime*100)*10;//装载独立单元上报的时间
			UploadUnit[i].cnt = 0;
			UploadUnit[i].flag = 1; //需要上报，置1
		}
		else
			UploadUnit[i].flag =0;//不需上报，置0
	}
	Thread_Login(FOREVER, 0, 20, &Upload_Check);
}

/*******************************************************************************
函 数 名：	void Upload_Immediately(u8 unit,u8 cmd,u8* data)
功能说明： 	设备属性状态上报，马上上报，为了多条件联动而做
参	  数： 	unit：需要立即上报的单元号
			cmd:上报的属性号
			data:上报的数据区指针
返 回 值：	无
注    意:	无延时上报
*******************************************************************************/
void Upload_Immediately(u8 unit,u8 cmd,u8* data)
{
	u8 len = 0u;
	u8 udata[255]={0};
	
	len = Get_SingleUnit_AttrLen(unit,cmd);
	if(len > 0u)
	{
		udata[0] = COMPLETE;
		udata[1] = unit;
		udata[2] = cmd;
		udata[3] = len;
		memcpy(&udata[4],data,len);
		Msg_UploadUnit(0x00 , 0x00, len + 4, &udata[0]);	
	}
}

#ifndef   STM32L151 		//不是低功耗产品
/*******************************************************************************
函 数 名：	Upload_Idle
功能说明： 	空闲时间，设备属性状态上报，防止死机
参	  数： 	无
返 回 值：	无
注    意:	1h-2h 离散度1s-3600s
*******************************************************************************/
void Upload_Idle(void)
{
	u32 time = 3600000 + (GetRandomTime()%3600)*1000;
	Thread_Login(ONCEDELAY , 0, time, &Upload_Idle);
	Upload_Attr(0);
}
/*******************************************************************************
函 数 名：	SysTickUpload
功能说明： 	心跳包上报，由主机轮询，短帧上报 设备ID+流水号（5 bytes）
参	  数： 	无
返 回 值：	无
注    意:	无
*******************************************************************************/
void SysTickUpload(void)
{
	#if (COMMUNICATION_SHORT_FRAME == 1) //短帧心跳包
		u8 udata[10];
		(*(ShortCMDAim_t*)&udata).version = SYSTICK_SHORT_VERSION ; //上报短帧版本
		(*(ShortCMDAim_t*)&udata).userID = gSysData.userID ; 
		(*(ShortCMDAim_t*)&udata).sourceID = gSysData.deviceID ;
		(*(ShortCMDAim_t*)&udata).serialNum = ++gSysData.serialNum ;
	
		if(FALSE == short_put(&send433_shortfifo, udata, sizeof(ShortCMDAim_t)))//判断433队列是否溢出
			Fault_Upload(FAULT_2 , SEND_433_FIFO_OVER , NULL);//暂时不做处理 yanhuan adding 2015/10/10
	#else
		Msg_UploadUnit(0, 0x00, 0, NULL);//常规帧心跳包
	#endif 	
}
#endif 
/*******************************************************************************
函 数 名：	static void Upload_Attr(u8 unit)
功能说明： 	设备属性状态上报
参	  数： 	无
返 回 值：	无
*******************************************************************************/
static void Upload_Attr(u8 unit)
{
	u8 udata[255]={0};
	u8 i = UnitCnt_Get();
	while(i--)
	{				
		if(((unit)&&(i== unit||i ==0))||!unit) //unit为0获取所有单元属性
		{
			udata[0] = i; //单元号
			udata[5] = Get_ManyUnit_Attr((UnitPara_t*)udata, &udata[3], &udata[6]+udata[4]);
			udata[4] += udata[3];	
		}			
	}
	udata[4] ++; //增加通信标识位		
	Msg_UploadUnit(0, 0x00, udata[4], &udata[5]);	
}


/**************************Copyright BestFu 2014-05-14*************************/
