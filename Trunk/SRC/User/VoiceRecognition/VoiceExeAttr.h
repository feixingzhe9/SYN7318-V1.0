/***************************Copyright BestFu 2014-05-14*************************
文	件：	voiceExeAttr.h
说	明：	语音对外函数接口
编	译：	Keil uVision4 V4.54.0.0
版	本：	v1.0
编	写：	Unarty
日	期：	2014.09.01 
修　改：	暂无
*******************************************************************************/
#ifndef __VOICEEXEATTR_H
#define __VOICEEXEATTR_H

#include "UnitCfg.h"

typedef struct
{
	u8 num;
	u8 IdentificationMark;
	u8 buf[7];
}VoicePara_st;

extern const AttrExe_st VoiceAttrTab[];
extern VoicePara_st gVoicePara;

void VoiceParaInit(u8 unitID);
void VoiceEepromInit(u8 unitID);


u8 VoiceExe_AttrNum(void);

MsgResult_t Set_VoiceCmdAdd_Attr(UnitPara_t *pdata);
MsgResult_t Set_VoiceCmdAlter_Attr(UnitPara_t *pData);
MsgResult_t Get_VoiceCommand_Attr(UnitPara_t *pdara, u8 *rlen, u8 *rpara);
MsgResult_t Set_VoiceCmdDel_Attr(UnitPara_t *pdata);
MsgResult_t Get_VoiceID_Attr(UnitPara_t *pdata, u8 *rlen, u8 *rpara);
MsgResult_t Set_VoiceSenstivity_Attr(UnitPara_t *pdata);
MsgResult_t Get_VoiceSenstivity_Attr(UnitPara_t *pdata, u8 *rlen, u8 *rpara);
MsgResult_t Set_VoiceLead_Attr(UnitPara_t *pData);
MsgResult_t Get_VoiceLead_Attr(UnitPara_t *pData, u8 *rlen, u8 *rpara);
MsgResult_t Set_WorkMode_Attr(UnitPara_t *pData);
MsgResult_t Get_WorkMode_Attr(UnitPara_t *pData, u8 *rlen, u8 *rpara);
MsgResult_t Set_LeadTime_Attr(UnitPara_t *pData);
MsgResult_t Get_LeadTime_Attr(UnitPara_t *pData, u8 *rlen, u8 *rpara);
MsgResult_t Set_DormancyTime_Attr(UnitPara_t *pData);
MsgResult_t Get_DormancyTime_Attr(UnitPara_t *pData, u8 *rlen, u8 *rpara);
MsgResult_t Get_Power_Attr(UnitPara_t *pData, u8 *rlen, u8 *rpara);
MsgResult_t Get_Cap_Attr(UnitPara_t *pData, u8 *rlen, u8 *rpara);
#endif //VoiceExeAttr.h end
/**************************FILE*END********************************/
