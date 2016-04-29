/***************************Copyright BestFu 2014-05-14*************************
文	件：	VoiceRecognition.c
说	明：	语音识别逻辑层头文件
编	译：	Keil uVision4 V4.54.0.0
版	本：	v1.0
编	写：	Unarty
日	期：	2014.09.01 
修　改：	暂无
*******************************************************************************/
#ifndef __VOICERECOGNITION_H
#define __VOICERECOGNITION_H

#include "UnitCfg.h"

#define VOICE_CMD_NUM		(200)	//语音指令数	
#define UPDATE_NUM			(10)	//更新语句时，一次向串口发送语句的条数

#define PWRKEY_PORT		GPIO_A
#define PWRKEY_PIN		pin_4

#define CHIPREST_PORT		GPIO_A
#define CHIPREST_PIN		pin_5



#define VOICE_RECOGNTION_DATA_CLR	  0
#define SYN7318_BUFF_MAX_LEN	29

typedef enum
{
	VOICE_TOUCH = 0,	//语音与触摸引导
	TOUCH = 1,			//触摸引导
	ALWAYS_ACTIVATION = 2	//时时激活
}WorkMode_t;		//工作模式

typedef struct
{
	u32 add[7];						//语句存在标示，1 bit 代表一条；0：为不存在，1：存在
	u16 Total;						//语句的总长度
	u8 len[VOICE_CMD_NUM];			//每条语句的长度
	u8 num;							//存在语音识别语句的最大数，如1/2/6/8,存在语音识别语句，num=8；
}VoiceSave_t;

typedef struct
{
	char cmd[20];
}VoiceCmd_t;

typedef struct
{
	u8 Senstivity;
	u8 Speakers;	//发音人
	u8 Volume;		//音量
	u8 Intonation;	//语调
	u8 Speed;		//语速
	u8 WakeUP;		//唤醒名
	u8 workMode;	//工作模式：	0：语音&触摸前导模式，1：触摸前导模式，	2：无前导模式
	u8 LeadTime;	//前导时间
	u8 DormancyTime;//休眠时间
	u8 UpdateCmd;	//升级标志，0：升级完成；1：没有升级；用于判断是否升级来确定初始化时是否更新语句
}Voice_t;

typedef struct
{
	u8 id;
	u8 InitCmd;		//初始化标志
	u8 InitRdy;		//初始化标志
	u8 BeingRecognized;//正在识别
	u8 RecognitionIsTimeOut;//识别超时
	u8 LeadCmd;		//前导标识，0：需要说前导码，1：开始语音识别
	u8 PlayNum;		//剩余播报语音的条数
	u8 State;		//状态
	u8 cap;			//电池容量
	u8 DormancyCmd;	//休眠标志，1：休眠中；0：没有休眠
	u8 isWakeUp;
//	u8 StopPlayCmd;	//停止播报标志，1：需要停止播报，0：正常
}VoiceState_t;




//////////////////////////////////////////////////
#define UART_DATA_RDY_TIME	2
typedef struct
{
	u8 data[20];
	u8 num;
}RcvFrom7318_t;

typedef struct
{
	u8 timecnt;
	u8 dataRdyFlag;
	
}UartDataProcess_t;

typedef enum
{
	NO_DATA = 0,
	
	DataRcv,//从串口接收的数据
	IsStartRecognition,	//是开始识别指令
	IsVoicePlay,
	IsStopRecognition,
	
//	IsTouchAndVoiceMode,
//	IsTouchMode,
//	IsAlwaysActiveMode,
	IsRecognitionResult
	
}DataType_e;

//typedef enum
//{
//	VOICE_ACTIVATION,	//语音和触摸激活
//	TOUCH_ACTIVATION,	//触摸激活
//	ALWAYS_ACTIVATION	//时时激活
//}wordActivationMode_t;//工作激活方式

#define BUF_DATA_LEN	35
typedef struct StructNode
{
	u8 len;
	u8 data[BUF_DATA_LEN];
	DataType_e dataType;		//
	struct StructNode *Next;
}StructNode_t;

extern u8 SaveData_SYN7318(u8 *data,u8 len,DataType_e dataType);
extern void AnalyzeRcvData(void);
extern u8 GetDataFromChain(u8 *buf,DataType_e dataType);
extern void ClrItemFromBuf(DataType_e dataType);

extern u8 SYN7318BufLen;//串口接收链表里的数据帧数
extern void DataTestFun(void);//测试函数
extern void TestFunForSYN7318(void);//测试函数
extern u8 TestDataForSYN7318[20];



////////////////////////////////////////////////////////////////////////////




//匹配指令在EEPROM中数据存储空间格式,占用数据空间大小（u32),ID,str,ID,str,ID,str……str
#define VOICE_USER              0x5a                    //存储空间使用标志
#define VOICE_DATA_SIZE			1024	                //分配给语音存储空间大小
#define VOICE_DATA_START_ADDR	(0x6000)	            //语音指令存储空间已被使用空间大小首地址
#define VOICE_DATA_CMD_ADDR     (VOICE_DATA_START_ADDR + sizeof(VoiceSave_t))//语音指令存储首地址

#define VOICE_SET_ADDR			(0x5C00)				//语音设置内容存储地址
#define VOICE_LEAD_ID			(0x55)					//引导码ID

extern Voice_t gVoice;
extern VoiceState_t gVoiceState;

void Voice_Init(void);
void VoiceRecognition_Init(void);
void Voice_SetInit(void);
void Voice_SetSave(void);
void Voice_Enable(void);
void Voice_Disable(void);
MsgResult_t Voice_DataEarse(u8 exist);
MsgResult_t Voice_CmdAdd(VoiceCmd_t *pVoiceCmd,u8 len,u8 ID);
MsgResult_t Voice_LeadAdd(VoiceCmd_t *pVoiceCmd);
MsgResult_t Voice_CmdDel(u8 id);
void Voice_CmdUpdata(void);
void Voice_StopVoice(void);
void Voice_Reset(void);
void Voice_Reset_Cmd(void);
void StartRecognition(void);


#endif //voice.h
/**************************Copyright BestFu 2014-05-14*************************/
