/***************************Copyright BestFu 2015-09-30*************************
文   件:	WT588D.c
说   明:	WT588D芯片操作文件
编   译:	Keil uVision5 V4.54.0.0
版   本:	v1.0
作   者:	yanhuan
时   间:	2015.09.30 
修   改:	无
*******************************************************************************/
#include "WT588D.h"
//#include "voice.h"
#include "delay.h"
#include "Thread.h"
#include "GPIO.h"
#include "stm32f10x.h"
#include "sys.h"
#include "LED.h"

#include "VoiceRecognition.h"

#include "PowerStatus.h"

//Voice_t gVoice;
u8 gVoiceVolum = 7;


static u8 gWT588D_Cmd = 0;
u8 CS_Busy = 0 ;




/*******************************************************************************
函 数 名:  	WT588D_Init
功能说明:  	WT588D芯片的引脚初始化
参    数:  	无			
返 回 值:  	无
注    意：  无
*******************************************************************************/
void WT588D_PinInit(void)
{
	RCC->APB2ENR&=0xfffffffe;
	RCC->APB2ENR|=0x00000001;//开启AFIO时钟
	AFIO->MAPR&=0xfdffffff;
	AFIO->MAPR|=0x02000000;  //关闭JTAG，启用SW
	
	
	GPIOx_Cfg(CS_PORT, CS_PIN  , OUT_PP_2M);
	GPIOx_Set(CS_PORT, CS_PIN );
	GPIOx_Cfg(CLK_PORT, CLK_PIN , OUT_PP_2M);
	GPIOx_Set(CLK_PORT, CLK_PIN);
	GPIOx_Cfg(DATA_PORT, DATA_PIN , OUT_PP_2M);
	GPIOx_Set(DATA_PORT, DATA_PIN);
	
	GPIOx_Cfg(BUSY_PORT, BUSY_PIN , IN_FLOATING); //busy引脚输入
	Ex_NVIC_Config(BUSY_PORT, 8, RTIR); //需要使能的位,bit7,上升沿触发
	EXTI->PR = BUSY_PIN;          //清中断
	MY_NVIC_Init(0, 2, ((u8)0x17), 2);  //中断优先级1+0	  
	EXTI->IMR |= BUSY_PIN;     //开启外部中断
	
	CS_SET ;
	CLK_SET ;
	DATA_SET ;
}
/*******************************************************************************
函 数 名:  	Set_WT588D_Cmd
功能说明:  	设置语音指令
参    数:  	cmd-指令（地址）		
返 回 值:  	无
注    意：  无
*******************************************************************************/
void Set_WT588D_Cmd(u8 cmd)
{
	gWT588D_Cmd = cmd ;
}
/*******************************************************************************
函 数 名:  	Check_WT588D_State
功能说明:  	检测WT588D是否忙
参    数:  	无	
返 回 值:  	状态
注    意：  无
*******************************************************************************/
WT588D_Cmd_Reply_T Check_WT588D_State(void)
{
	if(0 == GPIOx_Get(BUSY_PORT,BUSY_PIN)) return BUSY_STATE ;//忙状态，不接收指令
	return SPKR_STATE ;
}
/*******************************************************************************
函 数 名:  	Send_WT588D_Cmd
功能说明:  	WT588D的指令入口
参    数:  	cmd-指令（地址）		
返 回 值:  	无
注    意：  无
*******************************************************************************/
void Send_WT588D_Cmd(void)
{
	u8 cmd,i ;
	cmd = gWT588D_Cmd ;
	CS_SET;
	delay_ms(20);
	CS_RST;
	delay_ms(5);
	for( i = 0 ; i < 8 ; i++ )
	{
			CLK_RST ;
			if( cmd & 0x01 )
				DATA_SET ;
			else
				DATA_RST ;
			delay_us(300);
			CLK_SET ;     //发送bit
			delay_us(300);
			cmd >>= 1;		
	}
	CS_SET;
	delay_ms(50);
}



/*******************************************************************************
函 数 名:  	Send_WT588D_Cmd_To_OK
功能说明:  	为了防止语音覆盖，加入busy脚的检测和延迟发送的机制
参    数:  	无	
返 回 值:  	无
注    意：  无
*******************************************************************************/
void Send_WT588D_Cmd_To_OK(void)
{
	if(SPKR_STATE == Check_WT588D_State()) //不忙
	{
		Thread_Logout(Send_WT588D_Cmd_To_OK);
		Send_WT588D_Cmd();
		
		if(gWT588D_Cmd < 0xE0 || gWT588D_Cmd > 0xE7) //语音播报，关闭语音识别
		{
////////////////////			Voice_OFF();			
		}

	}	
	else
		Thread_Login(ONCEDELAY , 0 , 100 ,Send_WT588D_Cmd_To_OK);		
}


/*******************************************************************************
函 数 名:  	Set_WT588D_Volume
功能说明:  	设置语音播报WT588D的音量
参    数:  	vol-音量大小		
返 回 值:  	无
注    意：  指令E0-E7，音量递增
*******************************************************************************/
void Set_WT588D_Volume(void)
{
	if(gVoiceVolum < 8) 
	{
//		Thread_Logout(LED_Shine);
		Set_WT588D_Cmd(0XE0 + gVoiceVolum);
		Send_WT588D_Cmd_To_OK();
//		Instruct_StateCheck();
	}
}



/*******************************************************************************
函 数 名:  	WT588D_Report
功能说明:  	语音播报的实现
参    数:  	cmd-语音地址	
返 回 值:  	无
注    意：  支持同时发送两条语音指令
*******************************************************************************/
void WT588D_Report(Report_Voice_T cmd)
{
		if((u8)cmd >= 0xE0 && (u8)cmd <= 0xE7) return ;
		
		Set_WT588D_Cmd(0XE0 + gVoiceVolum);
		Send_WT588D_Cmd_To_OK();
		Send_WT588D_Cmd_To_OK();
//		Thread_Logout(LED_Shine);
//		Thread_Logout(LED_Run);
//		Thread_Logout(LED_OFF);
//		Thread_Logout(Instruct_StateCheck);	
		Set_WT588D_Cmd(cmd);
		Send_WT588D_Cmd_To_OK();	
}

/*******************************************************************************
函 数 名:  	EXTI9_5_IRQHandler
功能说明:  	BUSY引脚外部中断处理函数
参    数:  	无
返 回 值:  	无
注    意：  在语音播报完毕，调用该函数 
*******************************************************************************/
void EXTI9_5_IRQHandler(void)
{
	EXTI->IMR &= ~(BUSY_PIN);     //屏蔽外部中断
	EXTI->PR = BUSY_PIN;          //清中断
//	Voice_ON();
//	Instruct_StateCheck();  //指示灯状态	
	EXTI->IMR |= BUSY_PIN;       //开启外部中断
	
}




/**************************Copyright BestFu 2015-09-30*************************/




