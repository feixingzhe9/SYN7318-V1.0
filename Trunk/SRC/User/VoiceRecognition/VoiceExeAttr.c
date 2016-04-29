/***************************Copyright BestFu 2014-05-14*************************
文	件：	voiceExeAttr.c
说	明：	语音对外函数接口
编	译：	Keil uVision4 V4.54.0.0
版	本：	v1.0
编	写：	Unarty
日	期：	2014.09.01 
修　改：	暂无
*******************************************************************************/
#include "VoiceExeAttr.h"
#include "VoiceRecognition.h"
#include "KeyUnit.h"
#include "VoicePlayAttr.h"
#include "PowerStatus.h"
#include "Lark7318.h"
#include "Thread.h"
#include "Lark7318DataAnalyze.h"
#include "WT588D.h"
VoicePara_st gVoicePara = {0};
extern RecognitionResult_t gRecognitionResult;

//对属性读写的接口列表
const AttrExe_st VoiceAttrTab[] = {
	{0x01, LEVEL_0, Set_VoiceCmdAdd_Attr     	, NULL					   	},
	{0x02, LEVEL_0, Set_VoiceCmdDel_Attr     	, NULL                     	},
    {0x03, LEVEL_1, NULL                    	, Get_VoiceID_Attr         	},
	{0x04, LEVEL_0, Set_VoiceSenstivity_Attr	, Get_VoiceSenstivity_Attr 	}, 
	{0x05, LEVEL_0, Set_VoiceLead_Attr			, Get_VoiceLead_Attr		},
	{0x07, LEVEL_0, Set_WorkMode_Attr	 	 	, Get_WorkMode_Attr			},
//	{0x08, LEVEL_0, Set_LeadTime_Attr	 	 	, Get_LeadTime_Attr			},
	{0x09, LEVEL_0, Set_DormancyTime_Attr	 	, Get_DormancyTime_Attr		},
	{0x10, LEVEL_0, Set_VoicePlay_Attr			, NULL						},
	{0x11, LEVEL_0, Set_StopPlay_Attr			, NULL						},
	{0x12, LEVEL_0, Set_Speakers_Attr			, Get_Speakers_Attr			},
	{0x13, LEVEL_0, Set_VoicePlayState_Attr		, Get_VoicePlayState_Attr	},
	{0x20, LEVEL_1, NULL					 	, Get_Power_Attr			},
	{0xC8, LEVEL_1, NULL					 	, Get_Cap_Attr				},
};



/*******************************************************************************
函 数 名:  void RS232ParaInit(u8 unitID)
功能说明:  计算RS232网关属性命令的个数
参    数:  NONE
返 回 值:  无
*******************************************************************************/
u8 VoiceExe_AttrNum(void)
{
	return ((sizeof(VoiceAttrTab))/(sizeof(VoiceAttrTab[0])));
}


/*******************************************************************************
函 数 名: 	VoiceParaInit
功能说明:  	语音单元的初始化
参    数:  	无
返 回 值:  	无
*******************************************************************************/
void VoiceParaInit(u8 unitID)
{

	
#if VOICE_RECOGNTION_DATA_CLR
	
	VoiceSave_t voiceAdd;
	u8 i=0;
	for(i=0;i<sizeof(voiceAdd.add);i++)
	{
		voiceAdd.add[i] = 0;
	}
	for(i=0;i<sizeof(voiceAdd.len);i++)
	{
		voiceAdd.len[i] = 0;
	}
	voiceAdd.Total = 0;
	voiceAdd.num = 0;
	EEPROM_Write(VOICE_DATA_START_ADDR, sizeof(VoiceSave_t), (u8*)&voiceAdd);	//
	
#endif
	VoiceSave_t voiceAdd;
	EEPROM_Read(VOICE_DATA_START_ADDR, sizeof(VoiceSave_t), (u8*)&voiceAdd);	//
	Voice_Init();
	WT588D_PinInit();
	Thread_Login(FOREVER,0,50,WorkModeFun);
	
	
#if NEED_ADD_CODE
	
	
#endif
}

/*******************************************************************************
函 数 名:  	VoiceEepromInit
功能说明:  	语音单元EEPROM参数的默认值
参    数:  	unitID-单元号
返 回 值:  	无
*******************************************************************************/
void VoiceEepromInit(u8 unitID)
{
//	Voice_SetInit();		//testfor7318
	
#if NEED_ADD_CODE
	

	
#endif
}

/*******************************************************************************
函 数 名：  Set_VoiceCmdAdd_Attr  
功能说明：  添加语音指令
参	  数:   pData:   单元号/属性号/参数长度/参数内容
返 回 值：  消息执行结果
*******************************************************************************/
MsgResult_t Set_VoiceCmdAdd_Attr(UnitPara_t *pData)
{  
	if (pData->len > (sizeof(VoiceCmd_t)+1))	//参数长度核对
	{
		return PARA_LEN_ERR;
	}
	
    return Voice_CmdAdd((VoiceCmd_t*)&pData->data[1],pData->len-2,pData->data[0]);
}

/*******************************************************************************
函 数 名：  Set_VoiceCmdDel_Attr  
功能说明： 	删除语音指令
参	  数:   pData[0]:   单元号
            pData[1]:   长度信息
			pData[2]:	指令ID
返 回 值：  TRUE/FALSE(不能再继续添加)
*******************************************************************************/
MsgResult_t Set_VoiceCmdDel_Attr(UnitPara_t *pData)
{  
    return Voice_CmdDel(pData->data[0]);
}

/*******************************************************************************
函 数 名：  Get_VoiceID_Attr  
功能说明：  获取语音识别
参	  数:   pData[0]:    单元号
            *rlen-返回参数长度
            *rpara-返回参数存放地址          
返 回 值：  TRUE/FALSE
*******************************************************************************/
MsgResult_t Get_VoiceID_Attr(UnitPara_t *pData, u8 *rlen, u8 *rpara)
{
    *rlen = 1;
//	*rpara = gVoiceState.id;
	*rpara = (u8)(gRecognitionResult.ciTiaoID);
	
    return COMPLETE;
}

/*******************************************************************************
函 数 名：  Set_VoiceSenstivity_Attr  
功能说明： 	设置语音识别的灵敏度
参	  数:   pData:   单元号/长度信息/指令ID
返 回 值：  TRUE/FALSE(不能再继续添加)
*******************************************************************************/
MsgResult_t Set_VoiceSenstivity_Attr(UnitPara_t *pData)
{
	if (pData->data[0] > 100)
	{
		return PARA_MEANING_ERR;
	}
	
	gVoice.Senstivity = pData->data[0];
	Voice_SetSave();
//	Voice_Reset_Cmd();
	
    return COMPLETE;
}

/*******************************************************************************
函 数 名：  Get_VoiceSenstivity_Attr  
功能说明：  获取语音识别的灵敏度
参	  数:   pData[0]:    单元号
            *rlen-返回参数长度
            *rpara-返回参数存放地址          
返 回 值：  TRUE/FALSE
*******************************************************************************/
MsgResult_t Get_VoiceSenstivity_Attr(UnitPara_t *pData, u8 *rlen, u8 *rpara)
{
	*rlen = 1;
	EEPROM_Read(VOICE_SET_ADDR, sizeof(Voice_t), (u8*)&gVoice);	
	*rpara = gVoice.Senstivity;

    return COMPLETE;
}

/*******************************************************************************
函 数 名：  Set_VoiceLead_Attr  
功能说明： 	设置语音前导码
参	  数:   pData[0]:   单元号/长度信息/指令ID
返 回 值：  TRUE/FALSE(不能再继续添加)
*******************************************************************************/
MsgResult_t Set_VoiceLead_Attr(UnitPara_t *pData)
{  
	if (pData->len > (sizeof(VoiceCmd_t)+1))	//参数长度核对
	{
		return PARA_LEN_ERR;
	}
	
    return Voice_CmdAdd((VoiceCmd_t*)&pData->data,pData->len-1,0);
}

/*******************************************************************************
函 数 名：  Get_VoiceLead_Attr 
功能说明：  获取语音前导码
参	  数:   pData[0]:    单元号
            *rlen-返回参数长度
            *rpara-返回参数存放地址          
返 回 值：  TRUE/FALSE
*******************************************************************************/
MsgResult_t Get_VoiceLead_Attr(UnitPara_t *pData, u8 *rlen, u8 *rpara)
{
	VoiceSave_t voiceAdd;
	
	if (gVoiceState.cap != 200)	//	当前电池供电
	{
		return VOICE_POWER_BAT;
	}
	EEPROM_Read(VOICE_DATA_START_ADDR, sizeof(VoiceSave_t), (u8*)&voiceAdd);	//获取语音存储表
	
	*rlen = voiceAdd.len[0];
	
	EEPROM_Read(VOICE_DATA_CMD_ADDR, *rlen, rpara);
	
	return COMPLETE;  
}

/*******************************************************************************
函 数 名：  Set_WorkMode_Attr  
功能说明： 	设置工作模式属性
参	  数:   pData[0]:   单元号/长度信息/指令ID
返 回 值：  TRUE/FALSE(不能再继续添加)
*******************************************************************************/
MsgResult_t Set_WorkMode_Attr(UnitPara_t *pData)
{  
	u8 OldWorkMode = gVoice.workMode;
	
	if (gVoiceState.cap != 200)	//	当前电池供电
	{
		return VOICE_POWER_BAT;
	}
	
	if (1 != pData->len)
	{
		return PARA_LEN_ERR;
	}
	else if (pData->data[0] > 2)
	{
		return PARA_MEANING_ERR;
	}
	
	gVoice.workMode = pData->data[0];
	if(TOUCH != gVoice.workMode)//时时激活模式
	{
		StartRecognition();
	}
	
	if(OldWorkMode != gVoice.workMode)
	{
		Voice_SetSave();
	}

    return COMPLETE;
}

/*******************************************************************************
函 数 名：  Get_WorkMode_Attr 
功能说明：  获取工作模式属性
参	  数:   pData[0]:    单元号
            *rlen-返回参数长度
            *rpara-返回参数存放地址          
返 回 值：  TRUE/FALSE
*******************************************************************************/
MsgResult_t Get_WorkMode_Attr(UnitPara_t *pData, u8 *rlen, u8 *rpara)
{
	if (gVoiceState.cap != 200)	//	当前电池供电
	{
		*rlen = 1;
		*rpara = TOUCH;
		return VOICE_POWER_BAT;
	}
	*rlen = 1;
	*rpara = gVoice.workMode;

    return COMPLETE;
}
/*******************************************************************************
函 数 名：  Set_LeadTime_Attr  
功能说明： 	设置前导时间属性
参	  数:   pData[0]:   单元号/长度信息/指令ID
返 回 值：  TRUE/FALSE(不能再继续添加)
*******************************************************************************/
//MsgResult_t Set_LeadTime_Attr(UnitPara_t *pData)
//{
//	if (1 != pData->len)
//	{
//		return PARA_LEN_ERR;
//	}
//	gVoice.LeadTime = pData->data[0];
//	Voice_SetSave();
//	
//	return COMPLETE;
//}
/*******************************************************************************
函 数 名：  Get_LeadTime_Attr 
功能说明：  获取前导时间属性
参	  数:   pData[0]:    单元号
            *rlen-返回参数长度
            *rpara-返回参数存放地址          
返 回 值：  TRUE/FALSE
*******************************************************************************/
//MsgResult_t Get_LeadTime_Attr(UnitPara_t *pData, u8 *rlen, u8 *rpara)
//{
//	*rlen = 1;
//	*rpara = gVoice.LeadTime;

//    return COMPLETE;
//}
/*******************************************************************************
函 数 名：  Set_DormancyTime_Attr  
功能说明： 	设置休眠时间属性
参	  数:   pData[0]:   单元号/长度信息/指令ID
返 回 值：  TRUE/FALSE(不能再继续添加)
*******************************************************************************/
MsgResult_t Set_DormancyTime_Attr(UnitPara_t *pData)
{
	if (1 != pData->len)
	{
		return PARA_LEN_ERR;
	}
	gVoice.DormancyTime = pData->data[0];
	Voice_SetSave();
	Voice_Reset_Cmd();
	
	return COMPLETE;
}
/*******************************************************************************
函 数 名：  Get_LeadTime_Attr 
功能说明：  获取休眠时间属性
参	  数:   pData[0]:    单元号
            *rlen-返回参数长度
            *rpara-返回参数存放地址          
返 回 值：  TRUE/FALSE
*******************************************************************************/
MsgResult_t Get_DormancyTime_Attr(UnitPara_t *pData, u8 *rlen, u8 *rpara)
{
	*rlen = 1;
	*rpara = gVoice.DormancyTime;

    return COMPLETE;
}
/*******************************************************************************
函 数 名：  Get_Power_Attr  
功能说明：  获取供电源
参	  数:   pData[0]:    单元号
            *rlen-返回参数长度
            *rpara-返回参数存放地址          
返 回 值：  TRUE/FALSE
*******************************************************************************/
MsgResult_t Get_Power_Attr(UnitPara_t *pData, u8 *rlen, u8 *rpara)
{
	*rlen = 1;
	if (gVoiceState.cap < 101)	//	当前电池供电
	{
		*rpara = 1;
	}
    else
	{
		*rpara = 0;
	}

    return COMPLETE;
}
/*******************************************************************************
函 数 名：  Get_Cap_Attr  
功能说明：  获取剩余容量
参	  数:   pData[0]:    单元号
            *rlen-返回参数长度
            *rpara-返回参数存放地址          
返 回 值：  TRUE/FALSE
*******************************************************************************/
MsgResult_t Get_Cap_Attr(UnitPara_t *pData, u8 *rlen, u8 *rpara)
{
    *rlen = 1;
	if (gVoiceState.cap < 101)	//	当前电池供电
	{
		*rpara = gVoiceState.cap;
	}
    else
	{
		*rpara = 100;
	}

    return COMPLETE;
}

/**************************Copyright BestFu 2014-05-14*************************/
