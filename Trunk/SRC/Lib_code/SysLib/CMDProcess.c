/***************************Copyright BestFu 2014-05-14*************************
文	件：    cmdProcess.c
说	明：    FIFO指令处理函数
编	译：    Keil uVision4 V4.54.0.0
版	本：    v1.0
编	写：    Unarty
日	期：    2013-11-01
修  改:     2014.12.28 Unarty 修改单元属性执行边界条件判断值，防止多单元执行时属性越界
			2014.01.08 Unarty 增加单元数值核对 防止访问越界
*******************************************************************************/
#include "fifo.h"
#include "command.h"
#include "cmdProcess.h"
#include "Object.h"
#include "UnitCfg.h"
#include "MsgPackage.h"
#include "UnitShare_Attr.h"
#include "SI4432.h"
#include "Thread.h"
#include "Instruct.h"
#include "SysTick.h"
#include "AttrEncrypt.h"
#include "DataEncrypt.h"
#include "Updata.h"

#if STM32L151 
extern  void StandbyCountReset(void);
#endif 

static u8 Msg_Process(u8 *sdata, u8 len, u8 level);
FIFO_t rcv433fifo;
FIFO_t send433fifo;

u8 rcvBuf[FIFOBUFSZE];
static u8 sendBuf[FIFOBUFSZE];

Process_t rcv_433;
Process_t send_433;

#if (COMMUNICATION_SHORT_FRAME == 1)
	FIFO_t rcv433_shortfifo;
	FIFO_t send433_shortfifo;  //433短帧处理流程
	
	u8 rcvshortBuf[ADDFIFOBUFSZ];
	static u8 sendshortBuf[ADDFIFOBUFSZ];
	
	Process_t rcv_433short;
	Process_t send_433short;
static u8 ShortMsg_Level(u8 version) ;	
static u8 ShortMsg_Process(u8 *data, u8 len, u8 level); //短帧消息处理 yanhuan adding 2015/12/29
#endif

static u8   Msg_ReturnType(u8 msgType);
static void DataSend_Thread(void);
static u8   Msg_Level(u32 object);		////用于通信匹配（Unarty 2014.08.07)

/*****************************************************************************
  函数名:  void sendFIFOInit(void)
功能说明: 	发送FIFO队列初始化
    参数:  	void
  返回值:  	void
*****************************************************************************/
void FIFOInit(void)
{
	fifo_Init(&rcv433fifo, rcvBuf, FIFOBUFSZE);
	fifo_Init(&send433fifo, sendBuf, FIFOBUFSZE);
	rcv_433.flag = 0;
	rcv_433.type = COMMON ;
	rcv_433.con = NORMAL;	//接收数据正常处理
	rcv_433.fun = Msg_Process;
if (1 == Get_Encrypt433Flag())
	rcv_433.Data_Get 	= EncryptCmd_get;
else
	rcv_433.Data_Get 	= cmd_get;
	
	send_433.flag  = 0;
	send_433.type = COMMON ; 
	send_433.con = MANY_P;	//多次处理
	send_433.fun = Si4432_Send;
if (1 == Get_Encrypt433Flag())
	send_433.Data_Get 	= EncryptCmd_get;
else
	send_433.Data_Get 	= cmd_get;

#if (COMMUNICATION_SHORT_FRAME == 1)//短帧处理队列 yanhuan adding 2015/12/28
	fifo_Init(&rcv433_shortfifo, rcvshortBuf, ADDFIFOBUFSZ);
	fifo_Init(&send433_shortfifo, sendshortBuf, ADDFIFOBUFSZ);	
	rcv_433short.flag = 0;
	rcv_433short.type = SHORT;
	rcv_433short.con = NORMAL;	//接收数据正常处理
	rcv_433short.fun = &ShortMsg_Process;
	rcv_433short.Data_Get = short_get ;	
	send_433short.flag = 0; 
	send_433short.type = SHORT;
	send_433short.con = MANY_P;	//多次处理
	send_433short.fun = &Si4432_Send;
	send_433short.Data_Get = short_get ;
#endif	
	Thread_Login(FOREVER, 0, 3, &DataSend_Thread);
}

/*****************************************************************************
  函数名:  	ProcessFun
功能说明: 	处理发送函数修改
	参数:  	rcveFun:	接收处理函数
			sendFun:	发送处理函数
  返回值:  	void
*****************************************************************************/
void ProcessSend_Fun(ProcessFunction sendFun)
{
	send_433.fun = sendFun;
}

/*****************************************************************************
  函数名:  	DataSend_Thread
功能说明: 	数据发送线程
	参数:  	void
  返回值:  	void
*****************************************************************************/
void DataSend_Thread(void)
{
	Process_FIFOData(&send433fifo, &send_433);		//处理发送FIFO
	
#if (COMMUNICATION_SHORT_FRAME == 1)
	Process_FIFOData(&send433_shortfifo, &send_433short);		//处理发送短帧FIFO
#endif
}

/*******************************************************************
函 数 名：	Send_FIFOData
功能说明： 	将433队列中的数据通过433发送出去
参	  数： 	fifo:	操作目标FIFO
			send:	将发送的数据内容
			fun:	发送函数
返 回 值：	无
*******************************************************************/
void Process_FIFOData(FIFO_t *fifo, Process_t *prc)
{
	switch (prc->flag)
	{
		case 0:
				if (0 == (prc->len = prc->Data_Get(prc->data, fifo))) //从FIFO中获取一条指令失败
				{
					break;
				}
#if (COMMUNICATION_SHORT_FRAME == 1)				
				prc->level = (prc->type == COMMON)? \
				   Msg_Level(((Communication_t*)&prc->data[HEAD_SIZE])->aim.object.id)\
			            	:ShortMsg_Level(prc->data[0]);
#else 
				prc->level = Msg_Level(((Communication_t*)&prc->data[HEAD_SIZE])->aim.object.id);
#endif
				prc->mean = (prc->con == MANY_P ? gSysData.sendCnt : 1);//获取数据处理次数				
		case 1:	if (FALSE == (prc->fun(prc->data, prc->len, prc->level)) || --prc->mean) //数据处理还未完成
				{
					prc->flag = 1;
					break;
				}	
		default:	
					UpdateRandomTime(); //更新随机时间   yanhuan adding 2015/12/02
					prc->flag = 0;
					break;
	}
}
#if (COMMUNICATION_SHORT_FRAME == 1)
/*******************************************************************
函 数 名：	ShortMsg_Process
功能说明： 	短帧消息处理函数
参	  数： 	pMsg:	消息内容
			*data;	参数区内容
返 回 值：	无
*******************************************************************/
u8 ShortMsg_Process(u8 *data, u8 len, u8 level)
{  
  ShortCommunication_t *pMsg = (ShortCommunication_t*)data;
	if(COMPLETE == ShortObject_Check(pMsg , len)) //短帧核对成功
	{
		switch(pMsg->aim.version)
		{
			case SYSTICK_SHORT_VERSION:
				break ;
			default:
				break ;
		}
	}
	return TRUE;
}
/*******************************************************************
函 数 名：	ShortMsg_Level
功能说明： 	短帧消息执行等级。主要用数据发送
参	  数： 	消息目标
返 回 值：	消息等级
*******************************************************************/
u8 ShortMsg_Level(u8 version)
{
	if (SYSTICK_SHORT_VERSION == version)	//数据上报
	{
		return 2;
	}
	return 0;
}	
#endif
/*******************************************************************
函 数 名：	Msg_Process
功能说明： 	消息处理函数
参	  数： 	pMsg:	消息内容
			*data;	参数区内容
返 回 值：	无
*******************************************************************/
u8 Msg_Process(u8 *sdata, u8 len, u8 level)
{
    MsgResult_t result; 
	Communication_t *pMsg;
	pMsg = (Communication_t*)&sdata[HEAD_SIZE];  
	if (1 == Get_Encrypt433Flag())
	{
		if (0 != DecryptData((u8*)pMsg))		//解密数据失败
		{
			return TRUE;
		}
		len = HEAD_SIZE+sizeof(Communication_t) + pMsg->para.len+CRC_SIZE+TAIL_SIZE;
		memcpy(sdata+len-TAIL_SIZE,(u8*)TailCMD,TAIL_SIZE);
	}
	//核对通信版本	pMsg->version;
	if ((result = Object_Check(sdata, len, &pMsg->aim, &pMsg->para, 
		             &sdata[HEAD_SIZE+sizeof(Communication_t)])) == COMPLETE) //目标核对成功
	{
		/*指令执行 pMsg->para ; data*/
		result =  Msg_Execute(&pMsg->para);
	    /*指令反馈*/
		if (result != NO_ACK)
		{
#if STM32L151 		//专用于低功耗产品， 休眠时间重置
			StandbyCountReset();
#endif 
			Instruct();		//信号指示
			if (WRITEWITHACK == pMsg->para.msgType							//写应答
				|| READWITHACK == pMsg->para.msgType						//读应答
				|| ((WRITESUC == pMsg->para.msgType 
					|| READSUC == pMsg->para.msgType) && result == COMPLETE) 	//写/读成功应答并且操作成功
				|| ((WRITEFAIL == pMsg->para.msgType 
					|| READFAIL == pMsg->para.msgType) && result != COMPLETE) 	//写/读失败应答并且操作失败
				)	//消息需要应答
			{	
				pMsg->para.msgType = Msg_ReturnType(pMsg->para.msgType);	//消息执行结果
				Msg_Feedback(pMsg);
			}
		}
	}
	return TRUE;
}

/*******************************************************************
函 数 名：	Msg_Execute
功能说明： 	消息执行
参	  数： 	data:	要执行的数据内容
			len;	数据长度
返 回 值：	TRUE(重复)/FALSE(不重复)
*******************************************************************/
MsgResult_t Msg_Execute(CMDPara_t *para)
{
	MsgResult_t result = NO_CMD; 
	const AttrExe_st *pAttrExe = UnitTab[para->unit].attrExe;
	u8 ExeAttrNum = 0u;
	u8 ackLen = 0;
	
	if (para->unit < UnitCnt_Get())	//指令单元没有越界	2014.01.08 Unarty add 防止访问越界
	{
		do{ //遍历属性列表
			if (pAttrExe->cmd == para->cmd)	//属性号匹配OK
			{
				/*写操作*/
				if (WRITEWITHACK == para->msgType 
					|| WRITENACK == para->msgType
					|| WRITESUC  == para->msgType	
					|| WRITEFAIL == para->msgType
					)
				{
					if (pAttrExe->pSet_Attr != NULL)
					{	
						if (!gUnitData[para->unit].able)	//单元被禁用
						{
							result = UNIT_DISABLE;
							Upload(para->unit);
						}
						else
						{
#if (DALAY_EXECUTE_EN > 0u)
							//消息过滤
							DlyMsgFilterProcess((UnitPara_t*)&para->unit,RELATE_MSG_DEL);		//
#endif
							result =  pAttrExe->pSet_Attr((UnitPara_t*)&para->unit);
							if (result == OWN_RETURN_PARA) //如果写入结里带返回参数
							{
								if (pAttrExe->cmd > 0xCF)
								{
									result = COMPLETE;
								}
								ackLen = para->len;
								memmove((&para->len) + 2, (&para->len) + 1, ackLen);  //V1.8 add 
							}
						}
					}
					else
					{
						result = NO_WRITE;	//没有写权限
					}
				}
				/*读操作*/
				else if (READWITHACK == para->msgType
						|| READSUC == para->msgType	
						|| READFAIL == para->msgType
						)
				{
					if (pAttrExe->pGet_Attr != NULL)
					{
						result = pAttrExe->pGet_Attr((UnitPara_t*)&para->unit, &ackLen, ((&para->len) + 2));//((&para->len) + 1));V1.8 Chang
					}
					else
					{
						result = NO_READ;	//没有读权限
					}
				}
				else
				{
					result = MSG_TYPE_ERR;	//消息类型错误
				}
				break;
			}
		}while (((++ExeAttrNum) < UnitTab[para->unit].ExeAttrNum())  //2016.01.29 jay modify
				&&((++pAttrExe)->cmd < 0x0100));
		/*while((pAttrExe->cmd < (++pAttrExe)->cmd)		//2014.12.28 Unarty Add
				&& (pAttrExe->cmd < 0x0100));*/
	}

	para->len = ackLen + 1;
	*((&para->len)+1) = result;		//V1.8 ADD
	
	return result;
}

/*******************************************************************
函 数 名：	Msg_ReturnType
功能说明： 	消息返回类型
参	  数： 	执行消息类型
返 回 值：	返回消息应答类型
*******************************************************************/
u8 Msg_ReturnType(u8 msgType)
{
	switch (msgType)
	{
		case WRITEWITHACK:
		case WRITESUC:
		case WRITEFAIL:
				return ACKWRITE;
		case READWITHACK:
		case READSUC:
		case READFAIL:
				return READACK;
		default : break;
	}
	return 0;
}	

/*******************************************************************
函 数 名：	Msg_Level
功能说明： 	消息执行等级。主要用数据发送
参	  数： 	消息目标
返 回 值：	消息等级
*******************************************************************/
u8 Msg_Level(u32 object)
{
	if (MSG_UPLOAD_ID == object)	//数据上报
	{
		return 2;
	}
	else if (DelayID == object)		//目标操作可以延时应答
	{
		return 3;
	}
	return 0;
}
/**************************Copyright BestFu 2014-05-14*************************/			
