/******************************Copyright BestFu 2016-03-10*****************************
�ļ���: Lark7318DataAnalyze.c
˵��  : �Դ�Lark7318���յ������ݽ��з���
����  : Keil uVision4 V5.0
�汾  : v1.0
��д  : xjx
����  : 2016.03.10
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

u8 recognitionErrFeedback[3]= {0xfc,0x00,0x01};//����ʶ������᷵��4���ֽ����ݣ�ǰ������һ���ģ�ȡǰ������Ϊ����У��
u8 recognitionHaveCmdIdFeedback[3] = {0xfc,0x00,0x06};	//����ʶ��������ID
u8 recognitionHaveNoCmdIdFeedback[3] = {0xfc,0x00,0x04};//����ʶ��û������ID

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
������	:	Lark7318Prepare
����˵��:	Lark7318׼������
����		:	��
����ֵ	:	��
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
������	:	Lark7318Init
����˵��:	Lark7318��ʼ��
����		:	��
����ֵ	:	��
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
������	:	AnalyzeRcvData
����˵��:	������Lark7318���ص�����
����		:	��
����ֵ	:	��
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
		
		if(len>3)//ʶ���������
		{				
			if(4 == len)//ʶ��ʱ��ʶ���ʶ���ڲ�����״̬�ش�
			{
				if(CompareData(recognitionErrFeedback,buf,3))	//�����ݽ��в���У��
				{
					
					/*ʶ������������״̬������*/
					if(buf[3]>2 && buf[3]<8)//3,4,5,6,7
					{
						gRecognitionResult.recognitionState = FALSE;//ʶ��ʧ��
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
								
								if(TOUCH != gVoice.workMode)//ʱʱ����ģʽ
								{
									StartRecognition();
								}
								else
								{
									
									LED_RedOn();
									Thread_Login(ONCEDELAY,0,2000,LED_RedOff);//����ʶ��ʧ�ܵ�LEDָʾ
								}
								gVoiceState.RecognitionIsTimeOut = TRUE;////ʶ��ʱ
							
								break;
							default: break;//��������˵���������󣬺������Ӳ���������		
						}
					}
					
					
					else if(0x21 == buf[3])//��ʼ�������ѻش������ѳɹ�
					{
//						gRecognitionResult.wakeUpState = TRUE;
					}
					else if(0x22 == buf[3])//��ʼ�������ѻش�������ʧ��
					{
//						gRecognitionResult.wakeUpState = FALSE;
					}
					
					
					else if(0x4f == buf[3])//����״̬
					{
						gVoiceState.State = FREE;
					}
					
					else if(0x4a == buf[3])//ģ���ʼ���ɹ�
					{
						gVoiceState.State = INIT_RDY;	//ģ���ʼ���ɹ���Ļش�
					  gVoiceState.InitRdy = TRUE;
					}
					
					
					else if(0x41 == buf[3])//���ܳɹ���ģ���յ���ȷ������֡
					{
						u8 test = 0;
						test++;
					}
					else if(0x45 == buf[3])//����ʧ�ܣ�ģ���յ����������֡
					{
						u8 test = 0;
						test++;
					}
						
					
					else if(0x42 == buf[3])//ģ�鴦������ʶ�������������״̬
					{
						
					}
					else if(0x49 == buf[3])//ģ�鴦��Mp3����״̬
					{
						
					}
					else if(0x4B == buf[3])//ģ�鴦�ڸ��´ʵ�״̬
					{
						
					}
					else if(0x4E == buf[3])//ģ�鴦�������ϳɲ���״̬
					{
						
					}
					
					else if(0x31 == buf[3])//�ʵ���³ɹ�
					{
						
					}
					else if(0x32 == buf[3])//�ʵ����ʧ��
					{
						
					}
					
					
					else if(0x91 == buf[3])//�ʵ���Դ��¼�ɹ�
					{
						
					}
					else if(0x92 == buf[3])//�ʵ���Դ��¼ʧ��
					{
						
					}
			
				}				
			}
					
			if(8 == len)//ǰ4���ǽ��ճɹ����յ���ȷ������֡�ش�����4���ǲ�ѯ�Ľ��������ֻ����4��
			{
				if(CompareData(recognitionErrFeedback,&buf[4],3))	//�����ݽ��в���У��
				{
					if(0x4f == buf[7])//����״̬
					{
						gVoiceState.State = FREE;
					}
					else if(0x42 == buf[7])//ģ�鴦������ʶ�������������״̬
					{
						
					}
					else if(0x49 == buf[7])//ģ�鴦��Mp3����״̬
					{
						
					}
					else if(0x4B == buf[7])//ģ�鴦�ڸ��´ʵ�״̬
					{
						
					}
					else if(0x4E == buf[7])//ģ�鴦�������ϳɲ���״̬
					{
						
					}
					
					if(0x42 != buf[7])//û�д�������ʶ��״̬
					{
						if(TOUCH != gVoice.workMode)//ʱʱ����ģʽ
						{
							StartRecognition();
						}
					}
					
				}
			}
			if(7 == len)		//���ؽ��������ID
			{
				if(CompareData(recognitionHaveNoCmdIdFeedback,buf,3))	//�����ݽ��в���У��
				{					
					gRecognitionResult.recognitionState = TRUE;//ʶ��ɹ�
					gRecognitionResult.haveCmdID = FALSE;//������ID
					gRecognitionResult.currentRecognitionIsRdy = TRUE;
					
					
					gRecognitionResult.matchDegree = buf[4];
					gRecognitionResult.ciTiaoID = buf[5];//����ID��8λ
					gRecognitionResult.ciTiaoID <<=8;
					gRecognitionResult.ciTiaoID += buf[6];//����ID��8λ
					
					recognitionResult[0] = buf[5];//����ID��8λ
					recognitionResult[1] = buf[6];//����ID��8λ
					recognitionResultLen = 2;
					
					if(gRecognitionResult.matchDegree >= gVoice.Senstivity)
					{
						if(SYN7318BufLen < SYN7318_BUFF_MAX_LEN)
						{
							SYN7318BufLen = SaveData_SYN7318(recognitionResult,recognitionResultLen,IsRecognitionResult);//ʶ�������
						}
						else
						{
							ClrItemFromBuf(IsRecognitionResult);
						}
						
						
						if(0 == gRecognitionResult.ciTiaoID)//��������
						{
							gVoiceState.isWakeUp = TRUE;
							WT588D_Report(PLEASE_CMD);
						}
						else if(TOUCH != gVoice.workMode)//ʱʱ����ģʽ
						{
							StartRecognition();
						}
					}
					
		
				}				
			}
			
					
			if(9 == len)//���ؽ��������ID
			{
				if(CompareData(recognitionHaveCmdIdFeedback,buf,3))	//�����ݽ��в���У��
				{
					gRecognitionResult.recognitionState = TRUE;//ʶ��ɹ�
					gRecognitionResult.haveCmdID = TRUE;//������ID
					gRecognitionResult.currentRecognitionIsRdy = TRUE;
					
					gRecognitionResult.matchDegree = buf[4];
					gRecognitionResult.ciTiaoID = buf[5];//����ID��8λ
					gRecognitionResult.ciTiaoID <<=8;
					gRecognitionResult.ciTiaoID += buf[6];//����ID��8λ
					
					gRecognitionResult.cmdID = buf[7];//����ID��8λ
					gRecognitionResult.cmdID <<= 8;
					gRecognitionResult.cmdID += buf[8];//����ID��8λ		

					recognitionResult[0] = buf[5];//����ID��8λ
					recognitionResult[1] = buf[6];//����ID��8λ
					recognitionResult[2] = buf[7];//����ID��8λ
					recognitionResult[3] = buf[8];//����ID��8λ
					recognitionResultLen = 4;
					
					if(gRecognitionResult.matchDegree >= gVoice.Senstivity)
					{
						if(SYN7318BufLen < SYN7318_BUFF_MAX_LEN)
						{
							SYN7318BufLen = SaveData_SYN7318(recognitionResult,recognitionResultLen,IsRecognitionResult);//ʶ�������
						}
						else
						{
							ClrItemFromBuf(IsRecognitionResult);
						}
						
						
						if(0 == gRecognitionResult.ciTiaoID)//��������
						{
							gVoiceState.isWakeUp = TRUE;
							WT588D_Report(PLEASE_CMD);
						}
						if(TOUCH != gVoice.workMode)//ʱʱ����ģʽ
						{
							StartRecognition();
						}
					}
					
				}				
			}			
		}
			
		/*��ʱ�������ݳ��ȣ���ֹ������������Ϊ0ʱ�Զ�������� ��ɷ���Խ��*/
		if(SYN7318BufLen > 0)
		{
			SYN7318BufLen--;
		}
	}	
}

/*******************************************************************
������	:	CompareData
����˵��:	�Ƚ������ȳ������Ƿ�һ��
����		:	u8 *data:��Ҫ�Ƚϵ�����1
	u8 len:	���ݳ���
����ֵ:	1:��� 		0:����
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
������	:	QueryStateLark7318
����˵��:	��Lark7318����һ����ѯ�����Ĺ���״̬������
����		:	��
����ֵ	:	
˵��		���������ϵ�5������Lark7318��ʼ���ɹ���ſ��Ե���
*******************************************************************/ 
void QueryStateSYN7318(void)
{
	u8 buf[4] = {0xfd,0x00,0x01,0x21};
	UART_Send(buf,4,2);	
}


/*******************************************************************
������	:	ResetRecogintionTimeOutCnt
����˵��:	���������������ʶ��ʱ�ļ�ʱ��
����		:	��
����ֵ	:	
˵��		��
*******************************************************************/ 
void ResetRecogintionTimeOutCnt(void)
{
	gRecognitionTimeOutCnt = 0; 
}


/*******************************************************************
������	:	QueryDataAndSendToLark7318
����˵��:	��ѯ�������ݲ����͸�Lark7318
����		:	��
����ֵ	:	
˵��		��
*******************************************************************/ 
void QueryDataAndSendToSYN7318(void)
{
	u8 len;
	u8 buf[20] = {0};
//	u8 i;

	/*��������*/
	if((0x49 != gVoiceState.State)&&(0x4b != gVoiceState.State)\
		&&(0x4e != gVoiceState.State)&&(0x42 != gVoiceState.State))
	{
		len = GetDataFromChain(buf,IsVoicePlay);			
		if(len > 0)
		{
			UART_Send(buf,len,2);
		}		
	}
	
	/*��ʼʶ��ָ��*/
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
	
	
		/*ֹͣʶ��ָ��*/
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
