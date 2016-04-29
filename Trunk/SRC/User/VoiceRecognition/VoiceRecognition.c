/***************************Copyright BestFu 2014-05-14*************************
文	件：	VoiceRecognition.c
说	明：	语音识别逻辑层
编	译：	Keil uVision4 V4.54.0.0
版	本：	v1.0
编	写：	Unarty
日	期：	2014.09.01 
修　改：	暂无
*******************************************************************************/
#include "GPIO.h"
#include "VoiceRecognition.h"
#include "VoiceExeAttr.h"
#include "VoicePlayAttr.h"
#include "VoicePlayUnit.h"
#include "stm32f10x.h"
#include "SysHard.h"
#include "Thread.h"
#include "LED.h"
#include "uart.h"	
#include "PowerStatus.h"
#include "PWM.h"
#include "Capacity.h"
#include "KeyUnit.h"
#include "delay.h"
#include "Key.h"
#include "Lark7318.h"
#include "KeyUnit.h"

#include "si4432.h"

Voice_t gVoice;
VoiceState_t gVoiceState;
static u8 gVoiceEnableFlag;			//语音使能标记

/*******************************************************************************
函 数 名:  	VoiceSet
功能说明:  	因语音识别模块没有记忆功能，需要重新设置发音人、音量、语调、语速等
参    数:  	无 			
返 回 值:  	无
*******************************************************************************/
void VoiceSet(void)
{
	EEPROM_Read(VOICE_SET_ADDR, sizeof(Voice_t), (u8*)&gVoice);	
	VoicePlayState(gVoice.Volume,gVoice.Intonation,gVoice.Speed);
	VoiceSperkers(gVoice.Speakers);
	Voice_Reset();
	KeyUnit_Init();
}

/*******************************************************************************
函 数 名:  	Voice_Init
功能说明:  	初始化语音识别
参    数:  	无 			
返 回 值:  	无
*******************************************************************************/
void Voice_Init(void)
{
	gVoiceEnableFlag = FALSE;
	
	PowerStatus_Init();
	EEPROM_Read(VOICE_SET_ADDR, sizeof(Voice_t), (u8*)&gVoice);	
	
	
	if(gVoice.Volume > 10)
	{
		Voice_SetInit();
	}
	if (BATTERY_POWER == PowerStatus_Get())	//如果为电池供电 
	{
		Capacity_Init();	//开启电量检测
	}
	else
	{
		gVoiceState.cap = 200;	//用电池电量为200表示:供电为外接电源
	}
	
	if(!gVoice.UpdateCmd)
	{
		gVoice.UpdateCmd = 1;
		Voice_SetSave();
		Thread_Login(ONCEDELAY , 0, 500, Voice_Reset);
		Thread_Login(ONCEDELAY , 0, 1000, KeyUnit_Init);
	}
	else
	{
		Thread_Login(ONCEDELAY , 0, 3000, VoiceSet);
	}
	
	gVoiceState.LeadCmd = 1;
	gVoiceState.State = FREE;
	PowerDownCountReset();	//休眠模式
	
//	LED_State(gVoice.workMode);
	Thread_Login(FOREVER , 0, 1000, PowerStatus_Get);

//	Thread_Login(FOREVER , 0, 500, &VoicePlay);
}

/*******************************************************************************
函 数 名:  	VoiceRecognition_Init()
功能说明:  	初始化语音设置值
参    数:  	无 			
返 回 值:  	无
*******************************************************************************/
void VoiceRecognition_Init(void)
{
	GPIOx_Cfg(PWRKEY_PORT, 	 PWRKEY_PIN, IN_UPDOWN);
	GPIOx_Cfg(CHIPREST_PORT, CHIPREST_PIN, IN_UPDOWN);
	GPIOx_Rst(PWRKEY_PORT,   PWRKEY_PIN);
	GPIOx_Set(CHIPREST_PORT, CHIPREST_PIN);
}
/*******************************************************************************
函 数 名:  	Voice_SetInit()
功能说明:  	初始化语音设置值
参    数:  	无 			
返 回 值:  	无
*******************************************************************************/
void Voice_SetInit(void)
{
	VoiceSave_t voiceAdd = {0};

  gVoice.Senstivity = 70;
	gVoice.Volume = 5;		//音量
	gVoice.Intonation = 5;	//语调
	gVoice.Speed = 5;		//语速
	gVoice.Speakers = 0;	//发音人
	gVoice.workMode = VOICE_TOUCH;	//工作模式默认在触摸引导模式
	gVoice.DormancyTime = 15;
	VoiceSperkers(gVoice.Speakers);
	VoicePlayState(gVoice.Volume,gVoice.Intonation,gVoice.Speed);
	Voice_SetSave();

  EEPROM_Write(VOICE_DATA_START_ADDR, sizeof(VoiceSave_t), (u8*)&voiceAdd);
}

/*******************************************************************************
函 数 名:  	Voice_SetSave()
功能说明:  	语音设置值EEPROM存储
参    数:  	无 			
返 回 值:  	无
*******************************************************************************/
void Voice_SetSave(void)
{
	EEPROM_Write(VOICE_SET_ADDR, sizeof(Voice_t), (u8*)&gVoice);	
}

/*******************************************************************************
函 数 名:  	Voice_StopVoice
功能说明:  	停止识别
参    数:  	无 			
返 回 值:  	无
注	  意:
*******************************************************************************/
void Voice_StopVoice(void)
{
//	u8 buf[4] = {0xFD,0x00,0x01,0x1F};
	u8 buf[4] = {0xFD,0x00,0x01,0x11};
	
	UART_Send(buf, 4, 2);
}
/*******************************************************************************
函 数 名:  	WakeThread
功能说明:  	唤醒线程
参    数:  	无 			
返 回 值:  	无
注	  意：	会更新全
*******************************************************************************/
void WakeThread(void)
{
	u8 updatebuf[5] = {0xFD,0x00,0x02,0x13,0x01};
	
	UART_Send(updatebuf, 5, 2);	//识别词条缓存更新命令


	Thread_Wake(Key_process);
//	EXTI->IMR |= (nIRQ_PIN);		//打开433中断
	Si4432_Init();
}
/*******************************************************************************
函 数 名:  	VoiceCmdUpdata
功能说明:  	更新语音指令
参    数:  	无 			
返 回 值:  	无
注	  意：	会更新全
*******************************************************************************/
void VoiceCmdUpdata(void)
{
	u8 i;
	u8 buf[21] = {0xFD,0x00,0x00,0x12,0x00};
//	u8 buf[21] = {0xFD,0x00,0x00,0x1F,0x00};
//	u8 updatebuf[5] = {0xFD,0x00,0x02,0x13,0x01};
	u8 updatebuf[6] = {0xFD,0x00,0x03,0x13,0x00,0x00};
	VoiceSave_t voiceAdd;
	
	EEPROM_Read(VOICE_DATA_START_ADDR, sizeof(VoiceSave_t), (u8*)&voiceAdd);	//获取语音存储表
	
	voiceAdd.Total += voiceAdd.num + 3;
	
	buf[1] = (u8)(voiceAdd.Total>>8);
	buf[2] = (u8)voiceAdd.Total;
//	buf[1] = (u8)((voiceAdd.Total+3)>>8);
//	buf[2] = (u8)(voiceAdd.Total+3);
	
	UART_Send(buf, 5, 2);
	
	for(i=0;i<voiceAdd.num+1;i++)
	{
		if (0 != (voiceAdd.add[i>>5]&(1<<(i&0x1F))))	//有存储数据
		{
			EEPROM_Read(VOICE_DATA_CMD_ADDR + i*sizeof(VoiceCmd_t), voiceAdd.len[i], buf);
			buf[voiceAdd.len[i]] = 0x7C;
			UART_Send(buf, voiceAdd.len[i]+1, 2);
		}
		else
		{
			buf[0] = 0x7C;
			UART_Send(buf, voiceAdd.len[i]+1, 2);
		}
	}
	delay_ms(100);
	UART_Send(updatebuf, 6, 2);	//识别词条缓存更新命令

	Thread_Wake(Key_process);
	
//	EXTI->IMR |= (nIRQ_PIN);		//打开433中断
	Si4432_Init();
//	Thread_Login(ONCEDELAY , 0, 100, &WakeThread);

}

/*******************************************************************************
函 数 名:  	Voice_CmdUpdata
功能说明:  	更新语音指令缓冲区内容
参    数:  	无 			
返 回 值:  	无
注	  意：	会更新全
*******************************************************************************/
void Voice_CmdUpdata(void)
{
	u8 res = 0xff;
	VoiceSave_t voiceAdd;
	
	res = EEPROM_Read(VOICE_DATA_START_ADDR, sizeof(VoiceSave_t), (u8*)&voiceAdd);	//获取语音存储表
	
	if(0 != res)
	{
		if((0 == voiceAdd.num) || (voiceAdd.num == 0xFF))
		{
			VoiceSet();
			if(voiceAdd.num == 0xFF)
			{
				Voice_DataEarse(0);
			}
		}
		else
		{
			EXTI->IMR &= ~(nIRQ_PIN);		//屏蔽433中断


			Thread_Hang(Key_process);
			Thread_Hang(ImmediatelyDormancy);
			Thread_Hang(VoiceDormancy);
			Voice_StopVoice();
			Thread_Login(ONCEDELAY , 0, 100, &VoiceCmdUpdata);
		}
	}
}

/*******************************************************************************
函 数 名:  	Voice_CmdAdd
功能说明:  	语音指令添加
参    数:  	pVoiceCmd: 语音指令码: ID/内容  			
返 回 值:  	消息执行结果
*******************************************************************************/
MsgResult_t Voice_CmdAdd(VoiceCmd_t *pVoiceCmd,u8 len,u8 ID)
{
	VoiceSave_t voiceAdd;
	
	Thread_Login(ONCEDELAY, 0, 500, Voice_CmdUpdata);	//延时更新语音指令Buf

	EEPROM_Read(VOICE_DATA_START_ADDR, sizeof(VoiceSave_t), (u8*)&voiceAdd);	//获取语音存储表

	voiceAdd.add[ID>>5] |= (1<<(ID&0x1F));		//存储标识置1
	
	voiceAdd.Total = voiceAdd.Total - voiceAdd.len[ID] + (u16)len;
	voiceAdd.len[ID] = len;
	if(voiceAdd.num < ID)
	{
		voiceAdd.num = ID;
	}
	
//	gVoiceState.State = ADD;
	
	EEPROM_Write(VOICE_DATA_CMD_ADDR + ID*sizeof(VoiceCmd_t), 	//存储新指令
								voiceAdd.len[ID], (u8*)pVoiceCmd);
	
//	Lark7318_FrameInfo(buf,8,0x01,0x01);
	return EEPROM_Write(VOICE_DATA_START_ADDR, sizeof(VoiceSave_t), (u8*)&voiceAdd); //存储修改后的存储标识
}
/*******************************************************************************
函 数 名:  	Voice_DataEarse
功能说明:  	语音数据清空
参    数:  	exist：前导是否存在，0：不存在，1：存在
返 回 值:  	消息执行结果
*******************************************************************************/
MsgResult_t Voice_DataEarse(u8 exist)
{
    VoiceSave_t voiceAdd = {0};
	u8 i;
//	u8 buf[8] = {0xC9,0xBE,0xB3,0xFD,0xB3,0xC9,0xB9,0xA6};//删除成功
	if(exist)
	{
		Thread_Login(ONCEDELAY, 0, 500, Voice_CmdUpdata);	//延时更新语音指令Buf
		EEPROM_Read(VOICE_DATA_START_ADDR, sizeof(VoiceSave_t), (u8*)&voiceAdd);	//获取语音存储表
		
		voiceAdd.add[0] = 0x01;
		for(i=1;i<7;i++)
		{
			voiceAdd.add[i] = 0x00;
		}
		for(i=1;i<VOICE_CMD_NUM;i++)
		{
			voiceAdd.len[i] = 0;
		}
		voiceAdd.num = 1;
		voiceAdd.Total = voiceAdd.len[0];
	}
//	Lark7318_FrameInfo(buf,8,0x01,0x01);
    return EEPROM_Write(VOICE_DATA_START_ADDR, sizeof(VoiceSave_t), (u8*)&voiceAdd);
}

/*******************************************************************************
函 数 名:  	Voice_CmdDel
功能说明:  	语音指令删除
参    数:  	id:	语音指令码ID 			
返 回 值:  	TRUE/FALSE
*******************************************************************************/
MsgResult_t Voice_CmdDel(u8 id)
{
	u8 i;
    VoiceSave_t voiceAdd;
//	u8 buf[8] = {0xC9,0xBE,0xB3,0xFD,0xB3,0xC9,0xB9,0xA6};//删除成功
	
	if (0 == id)
	{
		EEPROM_Read(VOICE_DATA_START_ADDR, sizeof(VoiceSave_t), (u8*)&voiceAdd);	//获取语音存储表
		
		if (0 != (voiceAdd.add[id>>5]&(1<<(id&0x1F))))	//有存储数据
		{
			return Voice_DataEarse(1);
		}
		else
		{
			return Voice_DataEarse(0);
		}
	}
	Thread_Login(ONCEDELAY, 0, 500, Voice_CmdUpdata);	//延时更新语音指令Buf
	EEPROM_Read(VOICE_DATA_START_ADDR, sizeof(VoiceSave_t), (u8*)&voiceAdd);	//获取语音存储表

	if (0 != (voiceAdd.add[id>>5]&(1<<(id&0x1F))))	//找到存储位置
	{
		voiceAdd.add[id>>5] &= ~(1<<(id&0x1F));
		
		voiceAdd.Total -= (u16)voiceAdd.len[id];
		voiceAdd.len[id] = 0;

		/*获取存在语音识别语句的最大数*/
		for(i=voiceAdd.num;i>0;i--)
		{
			if (0 != (voiceAdd.add[i>>5]&(1<<(i&0x1F))))	//有存储数据
			{
				voiceAdd.num = i;
				break;
			}
		}

//		Lark7318_FrameInfo(buf,8,0x01,0x01);
		
		return EEPROM_Write(VOICE_DATA_START_ADDR, sizeof(VoiceSave_t), (u8*)&voiceAdd); //存储修改后的存储标识
	}
	else
	{
		return COMPLETE;
	}
}

/*******************************************************************************
函 数 名:  	Voice_Reset
功能说明:  	语音重置
参    数:  	无			
返 回 值:  	无
*******************************************************************************/
void Voice_Reset()
{

}

/*******************************************************************************
函 数 名:  	Voice_Reset_Cmd
功能说明:  	语音重置
参    数:  	无			
返 回 值:  	无
*******************************************************************************/
void Voice_Reset_Cmd(void)
{

}




///*******************************************************************************
//函 数 名:  	Voice_Enable
//功能说明:  	语音启用
//参    数:  	无 			
//返 回 值:  	无
//*******************************************************************************/
//void Voice_Enable(void)
//{
//	if ((FALSE == gVoiceEnableFlag) 
//		&& (ALWAYS_ACTIVATION != gVoice.workMode)	//工作在引导模式
//		)
//	{
////		LED_State(NORMALCY);
//		gVoiceEnableFlag = TRUE;
////		Voice_Reset();		//重置语音
//	}
//	if(ALWAYS_ACTIVATION != gVoice.workMode)
//		Thread_Login(ONCEDELAY, 0, 10000, Voice_Disable);
//}

/*******************************************************************************
函 数 名:  	Voice_Disable
功能说明:  	语音禁用
参    数:  	无 			
返 回 值:  	无
*******************************************************************************/
void Voice_Disable(void)
{
//	LED_State(gVoice.workMode);
	gVoiceEnableFlag = FALSE;
}



/*******************************************************************************
函 数 名:  	StartRecognition
功能说明:  	开始语音识别入队
参    数:  	无 			
返 回 值:  	无
*******************************************************************************/
void StartRecognition(void)
{
	u8 buf[5] = {0xfd,0x00,0x02,0x10,0x00};//
	u8 len = 5;
	buf[4] = 0x00;//词典编号为0
	
	SYN7318BufLen = SaveData_SYN7318(buf,len,IsStartRecognition);//开始识别指令入队	
	gVoiceState.BeingRecognized = TRUE; //
	gVoiceState.RecognitionIsTimeOut = FALSE;//
}
/**************************Copyright BestFu 2014-05-14*************************/
