/***************************Copyright BestFu 2014-05-14*************************
文	件：    Scene&Group.h
说	明：    场景与组相关头文件
编	译：    Keil uVision4 V4.54.0.0
版	本：    v1.0
编	写：    Unarty
日	期：    2014.06.26 
修　改：	暂无
*******************************************************************************/
#ifndef __SCENEGROUP_H
#define __SCENEGROUP_H

#include "UnitCfg.h"

typedef struct
{
	u8 start;	//初始值
	u8 cond;	//条件1
}Condition_t;




#define CLEAN_SCENE_INDEX		0
typedef struct 
{
	u8 unit;			//单元号
	u8 area;			//区域号
	u8 sceneNum;	//场景号
	u8 sceneAddr;	//场景地址
}SceneIndexes_t;

typedef struct SceneStructNode
{
	u8 unit;			//单元号
	u8 area;			//区域号
	u8 sceneNum;	//场景号
	u8 sceneAddr;	//场景地址
	struct SceneStructNode *next;
}SceneStructNode_t;
typedef struct 
{
	u8 sceneAddr;
	SceneStructNode_t *previous;
}StructFindResult_t;


extern u8 gSceneTotalNum;

extern u8 gSceneBuff[SCENE_MAX_NUM][4];

extern u8 gSceneIndex[32];

MsgResult_t Set_Scene_Attr(UnitPara_t *pData);
MsgResult_t Get_Scene_Attr(UnitPara_t *pData, u8 *rlen, u8 *rpara);

MsgResult_t Set_Group_Attr(UnitPara_t *pData);
MsgResult_t Get_Group_Attr(UnitPara_t *pData, u8 *rlen, u8 *rpara);

u8 SaveScene(u8 unit, u8 area, u8 sceneNum, u8 sceneAddr);
u8 DelOneScene(u16 addr);
u8 FindScene(u8 unit, u8 area, u8 sceneNum);
void SceneBufInit(void);

#endif //Scene&Group.h end
/**************************Copyright BestFu 2014-05-14*************************/
