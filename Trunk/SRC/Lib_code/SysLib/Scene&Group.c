/***************************Copyright BestFu 2014-05-14*************************
文	件：    Scene&Group.c
说	明：    场景与组相关函数
编	译：    Keil uVision4 V4.54.0.0
版	本：    v1.0
编	写：    Unarty
日	期：    2014.06.26 
修　改：	暂无
*******************************************************************************/
#include "Scene&Group.h"
#include "UserData.h"
#include "object.h"
#include "SysHard.h"


#include "EEPROM.h"
#include <stdlib.h>



//typedef void (*Del_fun)(u8 unit, u8 area, u8 num);

//static void SceneGroup_Del(Condition_t *pUnitCon, Condition_t *pAreaCon, Condition_t *pNumCon, u32 valArea, Del_fun fun);

//SceneStructNode_t *SceneBuf;
u8 gSceneTotalNum;

u8 gSceneBuff[SCENE_MAX_NUM][4] = {0};

u8 gSceneIndex[32] = {0};

SceneIndexes_t SceneIndexes;

/*******************************************************************************
函 数 名:  	SceneBufInit
功能说明:  	保存场景链表初始化
参    数:  	无
返 回 值:  	无
*******************************************************************************/
void SceneBufInit(void)
{
	u8 i=0;
#if CLEAN_SCENE_INDEX
	WriteDataToEEPROMEx(SCENESAVEFLAG_START_ADD,32,0);	
#endif
//	u8 seat[SCENE_MAX_NUM / 8];
	
	
	
  ReadDataFromEEPROM(SCENESAVEFLAG_START_ADD, sizeof(gSceneIndex), gSceneIndex);    //获取存储标识
	
//	ReadDataFromEEPROM(SCENE_START_ADD,sizeof(gSceneTotalNum), &gSceneTotalNum);
	
	for(i=0;i<SCENE_MAX_NUM-1;i++)
	{
		if(gSceneIndex[i/8]&(1<<(i%8)))
		{
			ReadDataFromEEPROM(SCENEFLAG_START_ADD + 1 +SCENE_INDEXEX_SPACE*i,	
														sizeof(SceneIndexes_t),(u8 *)&SceneIndexes);   
			SaveScene(SceneIndexes.unit,SceneIndexes.area,SceneIndexes.sceneNum,i+1);
			gSceneTotalNum++;
		}			
	}
	
}
/*******************************************************************************
函 数 名:  	SaveScene
功能说明:  	保存一个场景到链表
参    数:  	u8 unit 单元号
						u8 area 区域号
						u8 sceneNum 场景号
						u8 sceneAddr 场景的存储地址
返 回 值:  	消息执行结果
*******************************************************************************/
u8 SaveScene(u8 unit, u8 area, u8 sceneNum, u8 sceneAddr)
{

	if(sceneAddr>0)//防止数组越界
	{
		gSceneBuff[sceneAddr-1][0] = unit;
		gSceneBuff[sceneAddr-1][1] = area;
		gSceneBuff[sceneAddr-1][2] = sceneNum;
		gSceneBuff[sceneAddr-1][3] = sceneAddr;
//		sceneIndex[(sceneAddr-1)/8] |= 1<<((sceneAddr-1)%8);
//		WriteDataToEEPROM(SCENESAVEFLAG_START_ADD + (sceneAddr-1) / 8, 1, &sceneIndex[(sceneAddr-1)/8]);
	}
	

///////////////////////////////////////	
#if 0
	SceneStructNode_t *head = NULL, *temp = NULL;
	head = SceneBuf;
	while(NULL != head->next)
	{
		head = head->next;
	}//找到表尾
	
	temp = (SceneStructNode_t*)malloc(sizeof(SceneStructNode_t));
	if(NULL == temp)//内存申请失败
	{
		return FALSE;
	}
	
	head->next = temp;
	temp->area = area;
	temp->sceneAddr =  sceneAddr;
	temp->sceneNum = sceneNum;
	temp->next = NULL;
#endif
/////////////////////////////////////
	
	return TRUE;
}

/*******************************************************************************
函 数 名:  	FindScene
功能说明:  	在链表中查找一个场景
参    数:  	u8 unit 单元号
						u8 area 区域号
						u8 sceneNum 场景号						
返 回 值:  	u8 sceneAddr 场景的存储地址
							 FALSE：查找失败
*******************************************************************************/
u8 FindScene(u8 unit, u8 area, u8 sceneNum)
{
	u8 i;
	for(i=0;i<SCENE_MAX_NUM-1;i++)
	{
		if((unit == gSceneBuff[i][0])&&(area == gSceneBuff[i][1]) && (sceneNum == gSceneBuff[i][2]))
		{
			return (u8)(i+1);
		}
	}
	return FALSE;
	
/////////////////////////////////////////////////////////
#if 0
	SceneStructNode_t *h = NULL, *p = NULL;
	StructFindResult_t *FindResult;
	
	FindResult->previous = NULL;
	FindResult->sceneAddr = 0;
	
	if(gSceneTotalNum > 0)//有场景，开始查找对应的场景
	{
		h = SceneBuf;
		p = SceneBuf;
		while(NULL != h->next)
		{
			p = h;
			h = h->next;
			if(unit == h->unit && area == h->area && sceneNum == h->sceneNum)//找到对应的场景
			{
				FindResult->sceneAddr = h->sceneAddr;
				FindResult->previous = p;//前一个指针
				return FindResult;
			}
		}
		
		/*运行到这里就只剩链表尾部没有检查了*/
		if(unit == h->unit && area == h->area && sceneNum == h->sceneNum)//找到对应的场景
		{
			FindResult->sceneAddr = h->sceneAddr;
			FindResult->previous = p;//前一个指针
			return FindResult;
		}
		
	}	
	
	return FindResult;//查找失败
#endif
/////////////////////////////////////////////////////////////
	
	
}

/*******************************************************************************
函 数 名:  	DelOneScene
功能说明:  	删除一个场景
参    数:  	u8 unit 单元号
						u8 area 区域号
						u8 sceneNum 场景号						
返 回 值:   要删除场景的存储位置
*******************************************************************************/
u8 DelOneScene(u16 addr)
{
//	u8 addr;
//	
//	addr = FindScene(unit,area,sceneNum);
	if(SCENEFLAG_DEFAULT_VALUE == addr)//未找到对应的场景
	{
		return FALSE;
	}
		
	gSceneBuff[addr-1][0] = 0;
	gSceneBuff[addr-1][1] = 0;
	gSceneBuff[addr-1][2] = 0;
	gSceneBuff[addr-1][3] = 0;
	
	gSceneIndex[(addr-1)/8] &= ~(1<<((addr-1)%8));
	WriteDataToEEPROM(SCENESAVEFLAG_START_ADD + (addr-1) / 8, 1, &gSceneIndex[(addr-1)/8]);
	
	WriteDataToEEPROMEx(SCENEFLAG_START_ADD + 1 + (addr-1)*SCENE_INDEXEX_SPACE,
													sizeof(SceneIndexes_t), SCENEFLAG_DEFAULT_VALUE);
	
	
	gSceneTotalNum--;
	

	
//////////////////////////////////////	
#if 0
	SceneStructNode_t *temp = NULL;
	StructFindResult_t *FindResult;
	
	FindResult = FindScene(unit,area,sceneNum);
	temp = FindResult->previous->next;
	FindResult->previous->next = FindResult->previous->next->next;
	
	free(temp);
	
	return FindResult->sceneAddr;
#endif
/////////////////////////////////////////////
	return TRUE;
}


/*******************************************************************************
函 数 名:  	Set_Scene_Attr
功能说明:  	设置场景
参    数:  	data[0]: 目标单元
			data[1]: 参数长度
返 回 值:  	消息执行结果
*******************************************************************************/
MsgResult_t Set_Scene_Attr(UnitPara_t *pData)
{
	u32 valArea;
	u8 area, i, j;

#if (FLASH_ENCRYPTION_EN > 0u)
	u32 temp =0x00;	
	temp = DecryptionFlash();		//增加代码混淆 jay add 2015.12.07
	pData->len >>= temp;
#endif
	area = Check_Area(&pData->data[1], &valArea);		//获取区域地址	
#if (FLASH_ENCRYPTION_EN > 0u)	
	area += temp;
#endif
	
	if (pData->len < 3)				//参数长度错误
	{
		return PARA_LEN_ERR;
	}
#if (FLASH_ENCRYPTION_EN > 0u)	
	else if (((u8)(pData->data[0] + 1 + (u8)temp) > UnitCnt_Get())	//单元号不正确	（注意：清除指令为：0xFF, 所以加1后变成了零，符合条件）
			|| ((u8)(pData->data[5] + 1 + (u8)temp) > AREA_SCENEFLAG_NUM)//场景号不正确
			)
#else
	else if (((u8)(pData->data[0] + 1) > UnitCnt_Get())	//单元号不正确	（注意：清除指令为：0xFF, 所以加1后变成了零，符合条件）
			|| ((u8)(pData->data[5] + 1) > AREA_SCENEFLAG_NUM)//场景号不正确
			)	
#endif
	{
		return PARA_MEANING_ERR;
	}
	
	if (pData->data[7] == 1)	//增加一个场景
	{
		if (0 == pData->data[0] || CLEAR == pData->data[0] || CLEAR == pData->data[5])		//参数内容错误
		{
			return PARA_MEANING_ERR;
		}
		else if (valArea&(1<<pData->data[0]))//单元属于此区域
		{
			*(u8*)&pData->data[8] = pData->len - 9;		// 获取场景参数长度
			if (pData->data[8] < SCENESAVE_DATA_SPACE)	//空间长度合法
			{
//				if ( pData->data[5] == 1)
//				{
//					*(u8*)&pData->data[8] = pData->len - 9;// 获取场景参数长度
//				}
#if (SCENCETIME_EN > 0u)
				if(0xA0 == pData->cmd)
				{
					
					
					/////////////////TEST///////////////////////
					SceneIndexes.area = area;
					SceneIndexes.sceneNum = pData->data[0];
					SceneIndexes.sceneNum = pData->data[5];
					//////////////////TEST//////////////////////
					
					
					
					CheckSceneTimeData(pData->data[0] , area, pData->data[5]);
					return ((MsgResult_t)SceneEEPROM_Set(pData->data[0] , area, pData->data[5], (u8*)&pData->data[8]));
				}
				else
				{
					return (MsgResult_t)SceneEEPROM_SetEx(pData->data[0], area, pData->data[5], (u8*)&pData->data[8]);
				}
#else
				return ((MsgResult_t)SceneEEPROM_Set(pData->data[0] , area, pData->data[5], (u8*)&pData->data[8]));
#endif
			}
			else
			{
				return PARA_LEN_ERR;
			}
		}
		else
		{
			return AREA_ERR;
		}
	}
	else if (pData->data[7] == 0) 			//删除场景
	{		
		if (CLEAR == pData->data[0])		//所有单元
		{
			if (0 == area)					//所有区域
			{
				if (CLEAR == pData->data[5])//所有场景
				{
					SceneEEPROM_Init();
				}
				else
				{
					for (i = 1; i < UnitCnt_Get(); i++)
					{
						if (valArea&(1<<i))
						{						
							SceneEEPROM_Clr(i, area, pData->data[5]);
						}
					}
				}
			}
			else							//确定区域
			{
				for (i = 1; i < UnitCnt_Get(); i++)
				{
					if (valArea&(1<<i))
					{
						if (CLEAR == pData->data[5])//所有场景
						{
							for (j = 0 ; j < AREA_SCENEFLAG_NUM; j++)
							{							
								SceneEEPROM_Clr(pData->data[0], area, j);
							}
						}
						else
						{	
							SceneEEPROM_Clr(i, area, pData->data[5]);
						}
					}
				}
			}
		}
		else if (valArea&(1<<pData->data[0]))
		{
			if (0 == area)					//所有区域
			{
				if (CLEAR == pData->data[5])	//所有场景
				{
					SceneEEPROMUnit_Init(pData->data[0]);
				}
				else
				{					
					SceneEEPROM_Clr(pData->data[0], area, pData->data[5]);
				}
			}
			else if (CLEAR == pData->data[5])	//所有场景
			{
				for (i = 0 ; i < AREA_SCENEFLAG_NUM; i++)
				{					
					SceneEEPROM_Clr(pData->data[0], area, i);
				}
			}
			else
			{			
				SceneEEPROM_Clr(pData->data[0], area, pData->data[5]);
			}
		}
		else
		{
			return AREA_ERR;
		}
	}
	
	return COMPLETE;	
}

/*******************************************************************************
函 数 名:  	Get_Scene_Attr
功能说明:  	获取组
参    数:  	data[0]: 目标单元
			data[1]: 参数长度
			rLen:	返回参数长度
			rpara:	返回参数内容
返 回 值:  	消息执行结果
*******************************************************************************/
MsgResult_t Get_Scene_Attr(UnitPara_t *pData, u8 *rlen, u8 *rpara)
{
	u8 	area;
	u32 valArea;
	
	area = Check_Area(&pData->data[1], &valArea);		//获取区域地址
	
	if (pData->len < 3)	//参数长度错误
	{
		return PARA_LEN_ERR;
	}
	else if ((pData->data[0] == 0)||(pData->data[0] >= UnitCnt_Get())	//单元号错误	
			 || (*(u16*)&pData->data[5] > AREA_SCENEFLAG_NUM)					//区域号错误
			)
	{
		return PARA_MEANING_ERR;
	}
	else if (valArea&(1<<pData->data[0]))//单元属于此区域
	{	
#if (SCENCETIME_EN > 0u)
		SceneEEPROM_GetEx(pData->data[0], area, pData->data[5], rpara);
#else
		SceneEEPROM_Get(pData->data[0], area, pData->data[5], rpara);	//获取到组号标识
#endif
		*rlen = rpara[0] + 1;
		if(rpara[0] > 0u)
		{
			rpara[0] = 1;
		}
		else
		{
			rpara[0] = 0;
		}	
//		memmove(&rpara[0], &rpara[1], *rlen);
	}
	else
	{
		*rlen = 0;
		return AREA_ERR;
	}
	
	return COMPLETE;
}

/*******************************************************************************
函 数 名:  	Set_Group_Attr
功能说明:  	设置组
参    数:  	data[0]: 目标单元
			data[1]: 参数长度
返 回 值:  	消息执行结果
*******************************************************************************/
MsgResult_t Set_Group_Attr(UnitPara_t *pData)
{
	u32 valArea;
	u8 area, i, j;
	
#if (FLASH_ENCRYPTION_EN > 0u)	
	u32 temp = 0x00;
	temp = DecryptionFlash();  //增加代码混淆 jay add 2015.12.07
	pData->len += temp;
#endif
	area = Check_Area(&pData->data[1], &valArea);		//获取区域地址
	
#if (FLASH_ENCRYPTION_EN > 0u)
	area |= temp;
#endif
	
	if (pData->len < 3)	//参数长度错误
	{
		return PARA_LEN_ERR;
	}
#if (FLASH_ENCRYPTION_EN > 0u)	
	else if (((u8)(pData->data[0] + 1 + (u8)temp) > UnitCnt_Get())	//单元号不正确	（注意：清除指令为：0xFF, 所以加1后变成了零，符合条件）
			|| ((u8)(pData->data[5] + 1 + (u8)temp) > AREA_GROUP_NUM)//场景号不正确
			)
#else
	else if (((u8)(pData->data[0] + 1) > UnitCnt_Get())	//单元号不正确	（注意：清除指令为：0xFF, 所以加1后变成了零，符合条件）
			|| ((u8)(pData->data[5] + 1) > AREA_GROUP_NUM)//场景号不正确
			)	
#endif
	{
		return PARA_MEANING_ERR;
	}
	
	if (pData->data[7] == 1)	//增加一个组
	{
		if (0 == pData->data[0] || CLEAR == pData->data[0] || CLEAR == pData->data[5])		//参数内容错误
		{
			return PARA_MEANING_ERR;
		}
		
		else if (valArea&(1<<pData->data[0]))//单元属于此区域
		{
			GroupEEPROM_Set(pData->data[0], area, pData->data[5]);
		}
		else
		{
			return AREA_ERR;
		}
	}
	else if (pData->data[7] == 0) //删除组
	{			
		if (CLEAR == pData->data[0])		//所有单元
		{
			if (0 == area)					//所有区域
			{
				if (CLEAR == pData->data[5])//所有组
				{
					GroupEEPROM_Init();
				}
				else
				{
					for (i = 1; i < UnitCnt_Get(); i++)
					{
						if (valArea&(1<<i))
						{
							GroupEEPROM_Clr(i, area, pData->data[5]);
						}
					}
				}
			}
			else							//确定区域
			{
				for (i = 1; i < UnitCnt_Get(); i++)
				{
					if (valArea&(1<<i))
					{
						if (CLEAR == pData->data[5])//所有场景
						{
							for (j = 0 ; j < AREA_SCENEFLAG_NUM; j++)
							{
								GroupEEPROM_Clr(i, area, j);
							}
						}
						else
						{
							GroupEEPROM_Clr(i, area, pData->data[5]);
						}
					}
				}
			}
		}
		else if (valArea&(1<<pData->data[0]))	//单元区域正确
		{
			if (0 == area)					//所有区域
			{
				if (CLEAR == pData->data[5])	//所有场景
				{
					GroupEEPROMUnit_Init(pData->data[0]);
				}
				else
				{
					GroupEEPROM_Clr(pData->data[0], area, pData->data[5]);
				}
			}
			else if (CLEAR == pData->data[5])	//所有场景
			{
				for (i = 0 ; i < AREA_SCENEFLAG_NUM; i++)
				{
					GroupEEPROM_Clr(pData->data[0], area, i);
				}
			}
			else
			{
				GroupEEPROM_Clr(pData->data[0], area, pData->data[5]);
			}
		}
		else
		{
			return AREA_ERR;
		}
	}
	
	return COMPLETE;	
}

/*******************************************************************************
函 数 名:  	Get_Group_Attr
功能说明:  	获取组
参    数:  	pData->unit: 目标单元
			pData->cmd
			pData->len: 参数长度
			rLen:	返回参数长度
			rpara:	返回参数内容
返 回 值:  	消息执行结果
*******************************************************************************/
MsgResult_t Get_Group_Attr(UnitPara_t *pData, u8 *rlen, u8 *rpara)
{
	u8 	area;
	u32 valArea;
	
	area = Check_Area(&pData->data[1], &valArea);		//获取区域地址
	
	if (pData->len < 3)	//参数长度错误
	{
		return PARA_LEN_ERR;
	}
	else if ((pData->data[0] == 0)||(pData->data[0] >= UnitCnt_Get())	//单元号错误	
			 || (pData->data[5] < AREA_GROUP_NUM)						//区域号错误
			)
	{
		return PARA_MEANING_ERR;
	}
	else if (valArea&(1<<pData->data[0]))//单元属于此区域
	{
		*rlen = 1;
		*rpara = GroupEEPROM_Get(pData->data[0], area, pData->data[5]);
	}
	else
	{
		*rlen = 0;
		return AREA_ERR;
	}
	
	return COMPLETE;
}

///*******************************************************************************
//函 数 名：	Scene&Group_Del
//功能说明： 	场景与组的删除
//参	  数： 	pUnitCon:	单元删除条件
//			pAreaCon:	区域删除条件
//			pNumCon:	编号条件
//			valArea:	有效区域
//			Del_fun:	删除函数
//返 回 值：	无
//*******************************************************************************/
//static void SceneGroup_Del(Condition_t *pUnitCon, Condition_t *pAreaCon, Condition_t *pNumCon, u32 valArea, Del_fun fun)
//{
//	u8 i, j, k;
//	
//	for (i = pUnitCon->start; i < pUnitCon->cond; i++)
//	{
//		for (j = pAreaCon->start; j < pAreaCon->cond; j++)
//		{
//			for (k = pNumCon->start; k < pNumCon->cond; k++)
//			{
//				if (0 != (valArea&(1<<i)))	//单元属于此区域
//				{
//					fun(i, j, k);
//				}
//			}
//		}
//	}
//}
	
/**************************Copyright BestFu 2014-05-14*************************/	
