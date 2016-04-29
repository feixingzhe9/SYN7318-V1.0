/******************************Copyright BestFu 2016-03-10*****************************
文件名: Lark7318DataAnalyze.c
说明  : 对从Lark7318接收到的数据进行分析
编译  : Keil uVision4 V5.0
版本  : v1.0
编写  : xjx
日期  : 2016.03.10
*****************************************************************************/

#include "uart.h"	
#include "sys.h"

#include "Thread.h"
#include<stdlib.h>

#include "cmdProcess.h"
#include "thread.h"
#include "VoiceRecognition.h"
#include "KeyUnit.h"

#include "VoicePlayAttr.h"

#include "Lark7318DataAnalyze.h"
#include "LED.h"
#include "WT588D.h"


RecognitionResult_t gRecognitionResult;

u8 recognitionErrFeedback[3]= {0xfc,0x00,0x01};//语音识别错误后会返回4个字节数据，前三个是一样的，取前三个作为数据校验
u8 recognitionHaveCmdIdFeedback[3] = {0xfc,0x00,0x06};	//语音识别有命令ID
u8 recognitionHaveNoCmdIdFeedback[3] = {0xfc,0x00,0x04};//语音识别没有命令ID

u8 CompareData(u8 *data1,u8 *data2,u8 len);
u8 CheckRecognitionTimeOut(void);
void UartRcvProcess(void);
void SYN7318Prepare(void);
void SYN7318Init(void);


u8 testDataLen;
u8 TestDataForLark7318[20] = {0};


u8 gRecognitionTimeOutCnt = 0;
StructNode_t *SYN7318Buf = NULL;


/*******************************************************************
函数名	:	Lark7318Prepare
功能说明:	Lark7318准备工作
参数		:	无
返回值	:	无
*******************************************************************/ 
void SYN7318Prepare(void)
{
	static u8 timeCnt = 0;
	timeCnt++;
	if(timeCnt > 3 || TRUE == gVoiceState.InitRdy)
	{
		LED_GreenOn();//???
		Thread_Login(ONCEDELAY,0,1000,LED_GreenOff);
		Thread_Login(FOREVER,0,20,Key_process);
		Voice_StopVoice();

		Thread_Login(FOREVER,0,1000,QueryStateSYN7318);
		Thread_Login(FOREVER,0,50,QueryDataAndSendToSYN7318);		
		Thread_Logout(SYN7318Prepare);
	}
}

/*******************************************************************
函数名	:	Lark7318Init
功能说明:	Lark7318初始化
参数		:	无
返回值	:	无
*******************************************************************/ 
void SYN7318Init(void)
{
	u8 i = 0;
	
	SYN7318Buf = (StructNode_t *)malloc(sizeof(StructNode_t));
	SYN7318Buf->Next = NULL;
	SYN7318Buf->len = 0;
	SYN7318Buf->dataType = NO_DATA;
	for(i=0;i<20;i++)
	{
		SYN7318Buf->data[i] = 0;
	}
	
	Thread_Login(FOREVER,0,5,UartRcvProcess);//
	Thread_Login(FOREVER,0,100,AnalyzeRcvData);//
	
	Thread_Login(FOREVER,0,1000,SYN7318Prepare);		

}

/*******************************************************************
函数名	:	AnalyzeRcvData
功能说明:	分析从Lark7318返回的数据
参数		:	无
返回值	:	无
*******************************************************************/ 
void AnalyzeRcvData(void)
{
	u8 len;
	u8 buf[10] = {0};
	u8 i;
//	u8 workMode = gVoice.workMode;
	u8 recognitionResult[4] = {0};
	u8 recognitionResultLen = 0;
	
	len = GetDataFromChain(buf,DataRcv);
	testDataLen = len;
	for(i=0; i<len; i++)
	{
		TestDataForLark7318[i] = buf[i];
	}
	
	
	if(0 != len)
	{
		
		if(len>3)//识别错误的情况
		{				
			if(4 == len)//识别超时，识别拒识，内部错误，状态回传
			{
				if(CompareData(recognitionErrFeedback,buf,3))	//对数据进行部分校验
				{
					
					/*识别错误，清零各个状态和数据*/
					if(buf[3]>2 && buf[3]<8)//3,4,5,6,7
					{
						gRecognitionResult.recognitionState = FALSE;//识别失败
						gRecognitionResult.currentRecognitionIsRdy = FALSE;
						gRecognitionResult.haveCmdID = FALSE;
//						gRecognitionResult.wakeUpState = FALSE;
						
						switch(buf[3])
						{
										
							case 0x03:			
							case 0x04:	

							case 0x05:	
							case 0x06:
							case 0x07:
								
								if(TOUCH != gVoice.workMode)//时时激活模式
								{
									StartRecognition();
								}
								else
								{
									
									LED_RedOn();
									Thread_Login(ONCEDELAY,0,2000,LED_RedOff);//语音识别失败的LED指示
								}
								gVoiceState.RecognitionIsTimeOut = TRUE;////识别超时
							
								break;
							default: break;//到达这里说明参数错误，后期增加参数错误处理		
						}
					}
					
					
					else if(0x21 == buf[3])//开始语音唤醒回传：唤醒成功
					{
//						gRecognitionResult.wakeUpState = TRUE;
					}
					else if(0x22 == buf[3])//开始语音唤醒回传：唤醒失败
					{
//						gRecognitionResult.wakeUpState = FALSE;
					}
					
					
					else if(0x4f == buf[3])//空闲状态
					{
						gVoiceState.State = FREE;
					}
					
					else if(0x4a == buf[3])//模块初始化成功
					{
						gVoiceState.State = INIT_RDY;	//模块初始化成功后的回传
					  gVoiceState.InitRdy = TRUE;
					}
					
					
					else if(0x41 == buf[3])//接受成功，模块收到正确的命令帧
					{
						u8 test = 0;
						test++;
					}
					else if(0x45 == buf[3])//接受失败，模块收到错误的命令帧
					{
						u8 test = 0;
						test++;
					}
						
					
					else if(0x42 == buf[3])//模块处于语音识别或者语音唤醒状态
					{
						
					}
					else if(0x49 == buf[3])//模块处于Mp3播放状态
					{
						
					}
					else if(0x4B == buf[3])//模块处于更新词典状态
					{
						
					}
					else if(0x4E == buf[3])//模块处于语音合成播放状态
					{
						
					}
					
					else if(0x31 == buf[3])//词典更新成功
					{
						
					}
					else if(0x32 == buf[3])//词典更新失败
					{
						
					}
					
					
					else if(0x91 == buf[3])//词典资源烧录成功
					{
						
					}
					else if(0x92 == buf[3])//词典资源烧录失败
					{
						
					}
			
				}				
			}
					
			if(8 == len)//前4个是接收成功，收到正确的命令帧回传，后4个是查询的结果，这里只看后4个
			{
				if(CompareData(recognitionErrFeedback,&buf[4],3))	//对数据进行部分校验
				{
					if(0x4f == buf[7])//空闲状态
					{
						gVoiceState.State = FREE;
					}
					else if(0x42 == buf[7])//模块处于语音识别或者语音唤醒状态
					{
						
					}
					else if(0x49 == buf[7])//模块处于Mp3播放状态
					{
						
					}
					else if(0x4B == buf[7])//模块处于更新词典状态
					{
						
					}
					else if(0x4E == buf[7])//模块处于语音合成播放状态
					{
						
					}
					
					if(0x42 != buf[7])//没有处在语音识别状态
					{
						if(TOUCH != gVoice.workMode)//时时激活模式
						{
							StartRecognition();
						}
					}
					
				}
			}
			if(7 == len)		//返回结果无命令ID
			{
				if(CompareData(recognitionHaveNoCmdIdFeedback,buf,3))	//对数据进行部分校验
				{					
					gRecognitionResult.recognitionState = TRUE;//识别成功
					gRecognitionResult.haveCmdID = FALSE;//无命令ID
					gRecognitionResult.currentRecognitionIsRdy = TRUE;
					
					
					gRecognitionResult.matchDegree = buf[4];
					gRecognitionResult.ciTiaoID = buf[5];//词条ID高8位
					gRecognitionResult.ciTiaoID <<=8;
					gRecognitionResult.ciTiaoID += buf[6];//词条ID低8位
					
					recognitionResult[0] = buf[5];//词条ID高8位
					recognitionResult[1] = buf[6];//词条ID低8位
					recognitionResultLen = 2;
					
					if(gRecognitionResult.matchDegree >= gVoice.Senstivity)
					{
						if(SYN7318BufLen < SYN7318_BUFF_MAX_LEN)
						{
							SYN7318BufLen = SaveData_SYN7318(recognitionResult,recognitionResultLen,IsRecognitionResult);//识别结果入队
						}
						else
						{
							ClrItemFromBuf(IsRecognitionResult);
						}
						
						
						if(0 == gRecognitionResult.ciTiaoID)//是引导语
						{
							gVoiceState.isWakeUp = TRUE;
							WT588D_Report(PLEASE_CMD);
						}
						else if(TOUCH != gVoice.workMode)//时时激活模式
						{
							StartRecognition();
						}
					}
					
		
				}				
			}
			
					
			if(9 == len)//返回结果有命令ID
			{
				if(CompareData(recognitionHaveCmdIdFeedback,buf,3))	//对数据进行部分校验
				{
					gRecognitionResult.recognitionState = TRUE;//识别成功
					gRecognitionResult.haveCmdID = TRUE;//无命令ID
					gRecognitionResult.currentRecognitionIsRdy = TRUE;
					
					gRecognitionResult.matchDegree = buf[4];
					gRecognitionResult.ciTiaoID = buf[5];//词条ID高8位
					gRecognitionResult.ciTiaoID <<=8;
					gRecognitionResult.ciTiaoID += buf[6];//词条ID低8位
					
					gRecognitionResult.cmdID = buf[7];//命令ID高8位
					gRecognitionResult.cmdID <<= 8;
					gRecognitionResult.cmdID += buf[8];//命令ID低8位		

					recognitionResult[0] = buf[5];//词条ID高8位
					recognitionResult[1] = buf[6];//词条ID低8位
					recognitionResult[2] = buf[7];//词条ID高8位
					recognitionResult[3] = buf[8];//词条ID低8位
					recognitionResultLen = 4;
					
					if(gRecognitionResult.matchDegree >= gVoice.Senstivity)
					{
						if(SYN7318BufLen < SYN7318_BUFF_MAX_LEN)
						{
							SYN7318BufLen = SaveData_SYN7318(recognitionResult,recognitionResultLen,IsRecognitionResult);//识别结果入队
						}
						else
						{
							ClrItemFromBuf(IsRecognitionResult);
						}
						
						
						if(0 == gRecognitionResult.ciTiaoID)//是引导语
						{
							gVoiceState.isWakeUp = TRUE;
							WT588D_Report(PLEASE_CMD);
						}
						if(TOUCH != gVoice.workMode)//时时激活模式
						{
							StartRecognition();
						}
					}
					
				}				
			}			
		}
			
		/*及时更新数据长度，防止出现数据数量为0时仍对链表访问 造成访问越界*/
		if(SYN7318BufLen > 0)
		{
			SYN7318BufLen--;
		}
	}	
}

/*******************************************************************
函数名	:	CompareData
功能说明:	比较两个等长数组是否一致
参数		:	u8 *data:需要比较的数据1
	u8 len:	数据长度
返回值:	1:相等 		0:不等
*******************************************************************/ 
u8 CompareData(u8 *data1,u8 *data2,u8 len)
{
	u8 i;
	for(i=0;i<len;i++)
	{
		if(data1[i] != data2[i])
		{
			return 0;
		}		
	}
	return 1;
}


/*******************************************************************
函数名	:	QueryStateLark7318
功能说明:	向Lark7318发送一个查询所处的工作状态的命令
参数		:	无
返回值	:	
说明		：必须在上电5秒后或者Lark7318初始化成功后才可以调用
*******************************************************************/ 
void QueryStateSYN7318(void)
{
	u8 buf[4] = {0xfd,0x00,0x01,0x21};
	UART_Send(buf,4,2);	
}


/*******************************************************************
函数名	:	ResetRecogintionTimeOutCnt
功能说明:	清除用来计算语音识别超时的计时器
参数		:	无
返回值	:	
说明		：
*******************************************************************/ 
void ResetRecogintionTimeOutCnt(void)
{
	gRecognitionTimeOutCnt = 0; 
}


/*******************************************************************
函数名	:	QueryDataAndSendToLark7318
功能说明:	查询链表数据并发送给Lark7318
参数		:	无
返回值	:	
说明		：
*******************************************************************/ 
void QueryDataAndSendToSYN7318(void)
{
	u8 len;
	u8 buf[20] = {0};
//	u8 i;

	/*语音播报*/
	if((0x49 != gVoiceState.State)&&(0x4b != gVoiceState.State)\
		&&(0x4e != gVoiceState.State)&&(0x42 != gVoiceState.State))
	{
		len = GetDataFromChain(buf,IsVoicePlay);			
		if(len > 0)
		{
			UART_Send(buf,len,2);
		}		
	}
	
	/*开始识别指令*/
//	if((0x49 != gVoiceState.State)&&(0x4b != gVoiceState.State)\
//		&&(0x4e != gVoiceState.State)/*&&(0x42 != gVoiceState.State)*/)
	{
		len = GetDataFromChain(buf,IsStartRecognition);						
		if(len > 0)
		{
			LED_GreenOff();;Thread_Login(ONCEDELAY,0,800,LED_GreenOn);
			
			UART_Send(buf,len,2);
			
		}
	}
	
	
		/*停止识别指令*/
//	if((0x49 != gVoiceState.State)&&(0x4b != gVoiceState.State)\
//		&&(0x4e != gVoiceState.State)/*&&(0x42 != gVoiceState.State)*/)
	{
		len = GetDataFromChain(buf,IsStopRecognition);
		if(len > 0)
		{
			UART_Send(buf,len,2);
		}
	}
				
}


/****************File End***********Copyright BestFu 2014-05-14*****************/
