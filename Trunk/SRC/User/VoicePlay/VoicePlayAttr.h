#ifndef __VIOICEPLAYATTR_H
#define __VIOICEPLAYATTR_H

#include "UnitCfg.h"
#include "VoiceRecognition.h"

typedef enum
{

	PLAY			=1,	//语音模块正在播放
	ADD				=2,	//添加
	WARKUP			=3,	//唤醒
	RCV_SUCCEED			=4,	//成功
	RCV_FAILURE			=5,	//接收失败
	RECOGNITION		=6,	//开始识别
	IDENTIFY_SUCCED		=7,	//识别成功
	IDENTIFY_FAILED  =8,	//识别失败
	UPDATE_SUCCEED	=9,	//识别网络构建完成
	UPDATE_FAILURE	=10,//识别网络构建失败
	INIT_RDY = 11,
	FREE						//语音模块空闲中
}WORKMODE;

extern const AttrExe_st VoiceAttrTable[];

void voice_play_Init(void);

MsgResult_t Set_VoicePlay_Attr(UnitPara_t *pData);
MsgResult_t Set_StopPlay_Attr(UnitPara_t *pData);
void VoiceSperkers(u8 speakers);
MsgResult_t Set_Speakers_Attr(UnitPara_t *pData);
MsgResult_t Get_Speakers_Attr(UnitPara_t *pData, u8 *rlen, u8 *rpara);
void VoicePlayState(u8 Volume,u8 Intonation,u8 Speed);
MsgResult_t Set_VoicePlayState_Attr(UnitPara_t *pData);
MsgResult_t Get_VoicePlayState_Attr(UnitPara_t *pData, u8 *rlen, u8 *rpara);
#endif
