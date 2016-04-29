/***************************Copyright BestFu 2014-05-14*************************
文 件：  UserData.c
说 明：  用户数据管理文件
编 译：  Keil uVision4 V4.54.0.0
版 本：  v1.0
编 写：  Unarty
日 期：  2014.06.24
修　改：    2014.11.11 Uarty 修改UserData_Init中的初值判断，
            减少因EEPROM异常出现用户数据丢失的可能性。
*******************************************************************************/
//#include "UserData.h"
#include "EEPROM.h"
//#include "UnitCfg.h"
#include "Updata.h"
#include "SysHard.h"
#include "Upload.h"
#include "linkWRDC.h"
#include "FlashAttr.h"
#include "TimeHandle.h"



#include "Scene&Group.h"

SysData_t gSysData;


extern SceneIndexes_t SceneIndexes;



static void SceneEEPROMData_Init(void);
//static void SceneEEPROMData_Clr(u8 number);
static void SceneEEPROMData_Seat(u8 len, u8 *number);
static void SceneEEPROMData_Save(u32 addr, u8 *data);
static u32 SceneEEPROMSave_Seat(u8 len);
static u32 SceneEEPROMSave_Addr(u8 seat);
#if (SCENCETIME_EN > 0u)
static u32 SceneDlyTimeEEPROM_Set(u8 unit, u8 area, u8 num, u8 cnt, u8 *data);
static void SceneEEPROMData_ClrEx(u8 number);
static void  SceneEEPROMTimeFlag_Clr(u8 label);

#endif

/*******************************************************************************
函 数 名：  UserData_Init()
功能说明：  用户数据初始化
参   数：   无
返 回 值：  无
修   改：   Uarty 2014.11.11 增加Cnt，减少因EEPROM异常而导致数据删除。
*******************************************************************************/
void UserData_Init(void)
{
    static u8 cnt = 0;  // Unarty Add 2014.11.11

    ReadDataFromEEPROM(SYSDATA_START_ADD, sizeof(gSysData), (u8 *)&gSysData);   //获取初次使用标识
    if (HardID_Check(gSysData.deviceID) != 0)   								//设备ID不相同
    {
        Thread_Login(ONCEDELAY, 0, 50, UserData_Init);      					//反复验证，防止EEPROM上电时异常// Unarty Add 2014.11.11
        if (++cnt > 3)// Unarty Add 2014.11.11
        {
            Thread_Logout(UserData_Init);       								//去除返复验证接口// Unarty Add 2014.11.11
            FaultData_Init();
            UserEEPROMData_Init(FristLevel);        							//初始化用户EEPROM数据
        }
    }
    else if (cnt > 0)
    {
        cnt = 0;            													//Unarty 2014.12.02 修改，防止死锁。
        Unit_Init();
    }
}

/*******************************************************************************
函 数 名：  UserData_Init()
功能说明：  用户数据初始化
参   数：   FristLevel 不恢复433信道
            SecondLevel 恢复433信道
返 回 值：  无
*******************************************************************************/
void UserEEPROMData_Init(DataInitLevel_t level)
{
    u8 tmp[] = {0, 0xF8, 1, 0x53};
    SysEEPROMData_Init();
    UnitPublicEEPROMData_Init();
    UnitPrivateEEPROMData_Init();
    GroupEEPROM_Init();
    SceneEEPROM_Init();
    DelAllLinkFromTable(SIMPLESTLINK);

    if (2 == level)
    {
        Set_433Channel_Attr((UnitPara_t *)tmp);//恢复出厂设置时，恢复到83信道
    }

    Upload_Fault(USER_DATA_RESET);  			//用户数据重置
}

/*******************************************************************************
函 数 名：  FaultData_Init
功能说明：  异常事件存储空间初始化
参   数：   data:   异常返回存储地址
返 回 值：  无
*******************************************************************************/
void FaultData_Init(void)
{
    WriteDataToEEPROMEx(FAULT_START_ADD, FAULT_SPACE, 0);
}

/*******************************************************************************
函 数 名：  FaultData_Save
功能说明：  异常事件存储
参   数：   event： 异常事件名
返 回 值：  无
*******************************************************************************/
void FaultData_Save(u8 event)
{
    u32 name;

    ReadDataFromEEPROM(FAULT_START_ADD + sizeof(name)*event,
                       sizeof(name), (u8 *)&name);     //获取标识
    name++;
    WriteDataToEEPROM(FAULT_START_ADD + sizeof(name)*event,
                      sizeof(name), (u8 *)&name);     //获取标识
}

/*******************************************************************************
函 数 名：  FaultData_Get
功能说明：  异常事件存储
参   数：   data:   异常返回存储地址
返 回 值：  无
*******************************************************************************/
void FaultData_Get(u8 *len, u8 *data)
{
    *len = FAULT_SPACE;
    ReadDataFromEEPROM(FAULT_START_ADD, *len, data);    //获取标识
}

/*******************************************************************************
函 数 名：  SysEEPROMData_Init()
功能说明：  系统EEPROM数据初始化
参   数：   无
返 回 值：  无
*******************************************************************************/
void SysEEPROMData_Init(void)
{
    gSysData.userID     = DEFAULT_USERID;   //用户号
    gSysData.deviceID   = HardID_Check(0);      //
    gSysData.sendCnt    = 2; //默认两次发送信号，周旺修改，2015.7.2
    gSysData.serialNum  = 0;
    memset((u8 *)gSysData.BandAddr, 0, sizeof(gSysData.BandAddr)); //清绑定地址转发功能，周旺增加
    gSysData.BandAreaEn = 0; //清区域广播转发功能，周旺增加
    WriteDataToEEPROM(SYSDATA_START_ADD, sizeof(gSysData), (u8 *)&gSysData);
}

/*******************************************************************************
函 数 名：  SysEEPROMData_Init()
功能说明：  系统EEPROM数据初始化
参   数：   size:   设置数据大小
            data:   设置数据内容
返 回 值：  无
*******************************************************************************/
u8 SysEEPROMData_Set(u8 size , u8 *data)
{
    return WriteDataToEEPROM(SYSDATA_START_ADD + ((u32)data - (u32)&gSysData), size, data);
}

/*******************************************************************************
函 数 名：  UnitPrivateEEPROMData_Init
功能说明：  单元私有EEPROM数据初始化
参   数：   无
返 回 值：  无
*******************************************************************************/
void UnitPrivateEEPROMData_Init(void)
{
    u8 unitCnt;

    for (unitCnt = 0; unitCnt < UnitCnt_Get(); unitCnt++)
    {
        UnitPrivateEEPROMUnitData_Init(unitCnt);
    }
}

/*******************************************************************************
函 数 名：  UnitPrivateEEPROMUnitData_Init
功能说明：  单元私有EEPROM单元数据初始化
参   数：   unit： 目标格式化单元号
返 回 值：  无
*******************************************************************************/
void UnitPrivateEEPROMUnitData_Init(u8 unit)
{
    if (UnitTab[unit].DefEE != NULL)
    {
        UnitTab[unit].DefEE(unit);
    }
}

/*******************************************************************************
函 数 名：  UnitPublicEEPROMData_Init
功能说明：  单元共用EEPROM数据初始化
参   数：   无
返 回 值：  无
*******************************************************************************/
void UnitPublicEEPROMData_Init(void)
{
    u8 unitCnt;

    for (unitCnt = 0; unitCnt < UnitCnt_Get(); unitCnt++)
    {
        UnitPublicEEPROMUnitData_Init(unitCnt);
    }
}

/*******************************************************************************
函 数 名：  UnitPublicEEPROMUnitData_Init
功能说明：  单元共用EEPROM单元数据初始化
参   数：   unit： 目标格式化单元号
返 回 值：  无
*******************************************************************************/
void UnitPublicEEPROMUnitData_Init(u8 unit)
{
	*(u32*)gUnitData[unit].area = DEFAULT_AREA; //默认区域号
	gUnitData[unit].type = 0;					//默认设备应用类型
	gUnitData[unit].able = 1;					//默认使能标识
	gUnitData[unit].common = 0;				//默认单元应用大类
	gUnitData[unit].UploadAble = 1;   //默认上报使能标记   yanhuan adding 2016/01/04
	gUnitData[unit].UploadSetTime = 3;//默认上报上报使能最大随机时间
	WriteDataToEEPROM(UNITDATA_START_ADD + ONCEUNITDATA_SPACE*unit, 
						sizeof(UnitData_t), (u8*)&gUnitData[unit]);
}

/*******************************************************************************
函 数 名：  UnitPublicEEPROMData_Get
功能说明：  获取单元共用EEPROM数据
参   数：   unitCnt:    获取单元总数
            unitData:   单元数据内容
返 回 值：  无
*******************************************************************************/
void UnitPublicEEPROMData_Get(u8 unitCnt, UnitData_t *unitData)
{
    while (unitCnt--)
    {
        ReadDataFromEEPROM(UNITDATA_START_ADD + (ONCEUNITDATA_SPACE * (unitCnt)),
                           sizeof(UnitData_t), (u8 *)&unitData[unitCnt]);
    }
}

/*******************************************************************************
函 数 名：  UnitPublicEEPROMData_Set
功能说明：  设置单元共用EEPROM数据初始化
参   数：   size:   设置数据大小
            data:   设置数据内容
返 回 值：  无
*******************************************************************************/
u8 UnitPublicEEPROMData_Set(u8 size , u8 *data)
{
    u32 seat = (u32)data - (u32)&gUnitData;

    return WriteDataToEEPROM(UNITDATA_START_ADD + (ONCEUNITDATA_SPACE * (seat / sizeof(UnitData_t)))
                             + seat % sizeof(UnitData_t), size, data);
}

/*******************************************************************************
函 数 名：  GroupEEPROMData_Init
功能说明：  组EEPROM数据初始化
参   数：   无
返 回 值：  无
*******************************************************************************/
void GroupEEPROM_Init(void)
{
    u8  unitCnt;
    for (unitCnt = 1; unitCnt < UnitCnt_Get(); unitCnt++)
    {
        GroupEEPROMUnit_Init(unitCnt);
    }
}

/*******************************************************************************
函 数 名：  GroupEEPROMUnitData_Init
功能说明：  组EEPROM单元数据初始化
参   数：   unit： 目标格式化单元号
返 回 值：  无
*******************************************************************************/
void GroupEEPROMUnit_Init(u8 unit)
{
    WriteDataToEEPROMEx(GROUP_START_ADD + (UNIT_GROUP_SPACE * unit),
                        UNIT_GROUP_SPACE, 0);
}

/*******************************************************************************
函 数 名：  GroupEEPROM_Clr
功能说明：  清除组号标识位
参   数：   unit:  单元位置
            area： 区域位置(0~4)
            num:   组号
返 回 值：  无
*******************************************************************************/
void GroupEEPROM_Clr(u8 unit, u8 area, u8 num)
{
	u8 groupAddr[8];
	ReadDataFromEEPROM(GROUP_START_ADD + (UNIT_GROUP_SPACE * unit) + (AREA_GROUP_SPACE * area) + num/8,
                       1, &groupAddr[num/8]);
	if (num < AREA_GROUP_NUM && !(groupAddr[num/8] & (1 << (num%8)))) // 组号正确, 并且还未设置
	{
		groupAddr[num/8] &=  ~(1 << (num%8));
		WriteDataToEEPROM(GROUP_START_ADD + (UNIT_GROUP_SPACE * unit) + (AREA_GROUP_SPACE * area),
												1, &groupAddr[num/8]);
		
	}
	else if (num == CLEAR)//清除整个区域
	{
		u8 i=0;
		for(i=0;i<8;i++)
		{
			groupAddr[i] = 0;
			WriteDataToEEPROM(GROUP_START_ADD + (UNIT_GROUP_SPACE * unit) + (AREA_GROUP_SPACE * area),
												sizeof(groupAddr), groupAddr);
		}
	}
	

//////////////////////////////////////////////////////  
#if 0
		u16 flag;

    ReadDataFromEEPROM(GROUP_START_ADD + (UNIT_GROUP_SPACE * unit) + (AREA_GROUP_SPACE * area),
                       sizeof(flag), (u8 *)&flag);
    if (num < AREA_GROUP_NUM && (flag & (1 << num))) // 组号正确, 并且设置
    {
        flag &= ~(1 << num);
    }
    else if (num == CLEAR)
    {
        flag = 0;
    }
    else
    {
        return;
    }
    WriteDataToEEPROM(GROUP_START_ADD + (UNIT_GROUP_SPACE * unit) + (AREA_GROUP_SPACE * area),
                      sizeof(flag), (u8 *)&flag);
#endif
//////////////////////////////////////////////////////  
}

/*******************************************************************************
函 数 名：  GroupEEPROMData_Set
功能说明：  设置组号标识是否有效
参   数：   unit:  单元位置
            area： 区域位置(0~4)
            num:   组号
返 回 值：  无
*******************************************************************************/
void GroupEEPROM_Set(u8 unit, u8 area, u8 num)
{

	
		u8 groupAddr[8];

    ReadDataFromEEPROM(GROUP_START_ADD + (UNIT_GROUP_SPACE * unit) + (AREA_GROUP_SPACE * area) + num/8,
                       1, &groupAddr[num/8]);
    if (num < AREA_GROUP_NUM && !(groupAddr[num/8] & (1 << (num%8)))) // 组号正确, 并且还未设置
    {
        groupAddr[num/8] |=  1 << (num%8);
        WriteDataToEEPROM(GROUP_START_ADD + (UNIT_GROUP_SPACE * unit) + (AREA_GROUP_SPACE * area),
                          1, &groupAddr[num/8]);
    }
		
#if 0	
		u16 flag;
		ReadDataFromEEPROM(GROUP_START_ADD + (UNIT_GROUP_SPACE * unit) + (AREA_GROUP_SPACE * area),
                       sizeof(flag), (u8 *)&flag);
    if (num < AREA_GROUP_NUM && !(flag & (1 << num))) // 组号正确, 并且还未设置
    {
        flag |= (1 << num);
        WriteDataToEEPROM(GROUP_START_ADD + (UNIT_GROUP_SPACE * unit) + (AREA_GROUP_SPACE * area),
                          sizeof(flag), (u8 *)&flag);
    }
#endif
}

/*******************************************************************************
函 数 名：  GroupEEPROMData_Get
功能说明：  获取组号标识是否有效
参   数：   unit:  单元位置
            area： 区域位置(0~4)
            num:   组号
返 回 值：  标识结果 TRUE(有效)/FALSE(无效)
*******************************************************************************/
u8 GroupEEPROM_Get(u8 unit, u8 area, u8 num)
{
	
	u8 groupAddr[8];
	
//	ReadDataFromEEPROM(GROUP_START_ADD + (UNIT_GROUP_SPACE * unit) + (AREA_GROUP_SPACE * area),
//																									sizeof(flag), (u8 *)&flag);
	
	ReadDataFromEEPROM(GROUP_START_ADD + (UNIT_GROUP_SPACE * unit) + (AREA_GROUP_SPACE * area) + num/8,
                       1, &groupAddr[num/8]);
	if (groupAddr[num/8] & (1 << (num%8)))
	{
		return TRUE;
	}
	return FALSE;
	
	
	
	////////////////////////////////////////
#if 0
    u16 flag;

    ReadDataFromEEPROM(GROUP_START_ADD + (UNIT_GROUP_SPACE * unit) + (AREA_GROUP_SPACE * area),
                       sizeof(flag), (u8 *)&flag);
    if (flag & (1 << num))
    {
        return TRUE;
    }
		
		return FALSE;
#endif
 ////////////////////////////////////////////   
		
}

/*******************************************************************************
函 数 名：  SceneEEPROMData_Init
功能说明：  场景EEPROM数据初始化
参   数：   无
返 回 值：  无
*******************************************************************************/
void SceneEEPROM_Init(void)
{
//	u8  unitCnt;
	u16 sceneAddrCnt;
	u8 i=0;
	for(sceneAddrCnt=0;sceneAddrCnt<SCENE_MAX_NUM;sceneAddrCnt++)
	{
		for(i=0;i<4;i++)
		{
			gSceneBuff[sceneAddrCnt][i] = 0; 
		}	
	}
	WriteDataToEEPROMEx(SCENEFLAG_START_ADD,1 + SCENE_MAX_NUM*SCENE_INDEXEX_SPACE,SCENEFLAG_DEFAULT_VALUE);
	SceneEEPROMData_Init();

#if 0
	for (unitCnt = 1; unitCnt < UnitCnt_Get(); unitCnt++)
	{
			WriteDataToEEPROMEx(SCENEFLAG_START_ADD + (UNIT_SCENEFLAG_SPACE * unitCnt),
													UNIT_SCENEFLAG_SPACE, SCENEFLAG_DEFAULT_VALUE);
	}
	SceneEEPROMData_Init();
#endif
}

/*******************************************************************************
函 数 名：  SceneEEPROMUnitData_Init
功能说明：  场景EEPROM单元数据初始化
参   数：   unit： 目标格式化单元号
返 回 值：  无
*******************************************************************************/
void SceneEEPROMUnit_Init(u8 unit)
{
	u16 i=0;
	u16 cnt=0;
	u8 unitScene[SCENE_MAX_NUM];
	u8 scenetimedata[SCENE_MAX_NUM];
	for(i=0;i<SCENE_MAX_NUM;i++)
	{
		if(unit == gSceneBuff[i][0])
		{		
			DelOneScene(i+1);
			unitScene[cnt++] = i+1;
		}
	}
	ReadDataFromEEPROM(SCENCETIMFLAG_START_ADD, SCENE_MAX_NUM, scenetimedata);
	for (i = 0; i < cnt; i++)
	{
			if (unitScene[i] != SCENEFLAG_DEFAULT_VALUE)    //如果场景标识合法
			{
//					SceneEEPROMData_Clr(unitScene[i]);
#if (SCENCETIME_EN > 0u)
					if (SCENEFLAG_DEFAULT_VALUE != scenetimedata[unitScene[i]])
					{
							SceneEEPROMData_ClrEx(scenetimedata[unitScene[i]] - 1);
							scenetimedata[unitScene[i]] = SCENEFLAG_DEFAULT_VALUE;
					}
#endif
			}
	}
//	WriteDataToEEPROMEx(SCENEFLAG_START_ADD + (UNIT_SCENEFLAG_SPACE * unit),
//											UNIT_SCENEFLAG_SPACE, SCENEFLAG_DEFAULT_VALUE);
#if (SCENCETIME_EN > 0u)
	WriteDataToEEPROM(SCENCETIMFLAG_START_ADD, SCENE_MAX_NUM, scenetimedata);
#endif




	
	
#if 0
	u8 i, scene[UNIT_SCENEFLAG_SPACE], scenetimedata[SCENE_MAX_NUM];

	ReadDataFromEEPROM(SCENEFLAG_START_ADD + (UNIT_SCENEFLAG_SPACE * unit),
										 UNIT_SCENEFLAG_SPACE, scene);
	ReadDataFromEEPROM(SCENCETIMFLAG_START_ADD, SCENE_MAX_NUM, scenetimedata);
	for (i = 0; i < UNIT_SCENEFLAG_SPACE; i++)
	{
			if (scene[i] != SCENEFLAG_DEFAULT_VALUE)    //如果场景标识合法
			{
					SceneEEPROMData_Clr(scene[i]);
#if (SCENCETIME_EN > 0u)
					if (SCENEFLAG_DEFAULT_VALUE != scenetimedata[scene[i]])
					{
							SceneEEPROMData_ClrEx(scenetimedata[scene[i]] - 1);
							scenetimedata[scene[i]] = SCENEFLAG_DEFAULT_VALUE;
					}
#endif
			}
	}
	WriteDataToEEPROMEx(SCENEFLAG_START_ADD + (UNIT_SCENEFLAG_SPACE * unit),
											UNIT_SCENEFLAG_SPACE, SCENEFLAG_DEFAULT_VALUE);
#if (SCENCETIME_EN > 0u)
	WriteDataToEEPROM(SCENCETIMFLAG_START_ADD, SCENE_MAX_NUM, scenetimedata);
#endif
	
#endif
}

/*******************************************************************************
函 数 名：  SceneEEPROM_Clr
功能说明：  清除场景
参   数：   unit:  单元位置
            area： 区域位置(0~4)
            num:   场景号
返 回 值：  无
*******************************************************************************/
void SceneEEPROM_Clr(u8 unit, u8 area, u8 num)
{
	u8 label = 0u;							//场景存储块的标号
	

	label = FindScene(unit,area,num);
	
	if (SCENEFLAG_DEFAULT_VALUE != label)	//场景存储块的标号合法(非零)
	{
		DelOneScene(label);
		
#if (SCENCETIME_EN > 0u)
	SceneEEPROMTimeFlag_Clr(label);
#endif

	}



	
#if 0	
ReadDataFromEEPROM(SCENEFLAG_START_ADD + (UNIT_SCENEFLAG_SPACE * unit) + (AREA_SCENEFLAG_SPACE * area) + (SCENEFLAG_SPACE * num),
										 sizeof(label), (u8 *)&label);
	if (label != SCENEFLAG_DEFAULT_VALUE)	//场景存储块的标号合法(非零)
	{
			WriteDataToEEPROMEx(SCENEFLAG_START_ADD + (UNIT_SCENEFLAG_SPACE * unit) + (AREA_SCENEFLAG_SPACE * area) + (SCENEFLAG_SPACE * num),
													1, SCENEFLAG_DEFAULT_VALUE);
			SceneEEPROMData_Clr(label);
#if (SCENCETIME_EN > 0u)
	SceneEEPROMTimeFlag_Clr(label);
#endif

	}
#endif
}

/*******************************************************************************
函 数 名：  inline u32 Check_PositionRange(u8 position)
功能说明：  检查场景的起始存储位置
参   数：   position:待检查场景的起始存储位置的变量
返 回 值：  返回场景的起始存储位置的值
*******************************************************************************/
__inline u32 Check_PositionRange(u8 position)
{
    if (position < SCENE_SPACE1_NUM)
    {
        return 1;
    }
    else if (position < SCENE_SPACE2_NUM)
    {
        return (SCENE_SPACE1_NUM);
    }
    else
    {
        return (SCENE_SPACE1_NUM + SCENE_SPACE2_NUM);
    }
}

#if (SCENCETIME_EN > 0u)
/*******************************************************************************
函 数 名：  void CheckSceneTimeData(u8 unit, u8 area, u8 num)
功能说明：  检查场景的是否有时间数据
参   数：   unit:       单元位置
            area：      区域位置(0~4)
            num:        情景号
返 回 值：  无
说    明:	此函数用于无时间场景数据覆盖有时间场景数据时使用
*******************************************************************************/
void CheckSceneTimeData(u8 unit, u8 area, u8 num)
{
	u8 label = 0x00;	
	label = FindScene(unit,area,num);
//	ReadDataFromEEPROM(SCENEFLAG_START_ADD + 1 + (label-1)*SCENE_INDEXEX_SPACE + 3,
//                       sizeof(label), (u8 *)&label);
	if (label != SCENEFLAG_DEFAULT_VALUE)			// 场景标识正确
	{
		SceneEEPROMTimeFlag_Clr(label);
	}
	
#if 0
    ReadDataFromEEPROM(SCENEFLAG_START_ADD + (UNIT_SCENEFLAG_SPACE * unit) + (AREA_SCENEFLAG_SPACE * area) + (SCENEFLAG_SPACE * num),
                       sizeof(label), (u8 *)&label);
	if (label != SCENEFLAG_DEFAULT_VALUE)			// 场景标识正确
	{
		SceneEEPROMTimeFlag_Clr(label);
	}
#endif
}
#endif

/*******************************************************************************
函 数 名：  SceneEEPROM_Set
功能说明：  写入一个场景
参   数：   unit:       单元位置
            area：      区域位置(0~4)
            num:        情景号
            data[0]:    场景内容长度
            data[1~n]:  场景内容
返 回 值：  存储结果
*******************************************************************************/
u32 SceneEEPROM_Set(u8 unit, u8 area, u8 num, u8 *data)
{
	u8 flag = 0x00;	
	
	flag = FindScene(unit,area,num);
	
	if (Check_PositionRange(flag) != SceneEEPROMSave_Seat(data[0]))     //相同场景号的情况下,所需要的存储空间内存块不同,需要重新申请
    {
			DelOneScene(flag);
//       SceneEEPROMData_Clr(flag);                                      //清除之前存储标识
#if (SCENCETIME_EN > 0u)	
			SceneEEPROMTimeFlag_Clr(flag);
#endif
      flag = SCENEFLAG_DEFAULT_VALUE;
    }
		
		
	if(SCENEFLAG_DEFAULT_VALUE == flag)//未找到该场景，场景还没有申请空间
	{
		SceneEEPROMData_Seat(data[0], &flag);                           //申请一个存储空间
		if (flag == SCENEFLAG_DEFAULT_VALUE)
		{
			return EEPROM_RAND_ERR;
		}
		
		SaveScene(unit,area,num,flag);//
		
//		gSceneIndex[(flag-1)/8] |= 1<<((flag-1)%8);
//		WriteDataToEEPROM(SCENESAVEFLAG_START_ADD + (flag-1) / 8, 1, &gSceneIndex[(flag-1)/8]);
		
		WriteDataToEEPROM(SCENEFLAG_START_ADD + 1 +SCENE_INDEXEX_SPACE*(flag-1),	
														sizeof(gSceneBuff)/SCENE_MAX_NUM,(u8 *)&gSceneBuff[flag-1]);//
				
	}
	
	/////////////////TEST///////////////////////
					SceneIndexes.sceneAddr = flag;
	//////////////////TEST//////////////////////
	
	SceneEEPROMData_Save(SceneEEPROMSave_Addr(flag), data);
			
	
////////////////////////////////////////////////////////////////////	
#if 0
    ReadDataFromEEPROM(SCENEFLAG_START_ADD + (UNIT_SCENEFLAG_SPACE * unit) + (AREA_SCENEFLAG_SPACE * area) + (SCENEFLAG_SPACE * num),
                       sizeof(flag), (u8 *)&flag);
    if (Check_PositionRange(flag) != SceneEEPROMSave_Seat(data[0]))     //相同场景号的情况下,所需要的存储空间内存块不同,需要重新申请
    {
        SceneEEPROMData_Clr(flag);                                      //清除之前存储标识
#if (SCENCETIME_EN > 0u)	
		SceneEEPROMTimeFlag_Clr(flag);
#endif
        flag = SCENEFLAG_DEFAULT_VALUE;
    }
    if (flag == SCENEFLAG_DEFAULT_VALUE)                                //场景还没有申请空间
    {
        SceneEEPROMData_Seat(data[0], &flag);                           //申请一个存储空间
        if (flag == SCENEFLAG_DEFAULT_VALUE)
        {
            return EEPROM_RAND_ERR;
        }
        WriteDataToEEPROM(SCENEFLAG_START_ADD + (UNIT_SCENEFLAG_SPACE * unit) + (AREA_SCENEFLAG_SPACE * area) + (SCENEFLAG_SPACE * num),
                          sizeof(flag), (u8 *)&flag);
    }
    SceneEEPROMData_Save(SceneEEPROMSave_Addr(flag), data);
#endif		
////////////////////////////////////////////////////////////////////			
		
		
		
    return COMPLETE;
}

#if (SCENCETIME_EN > 0u)
/*******************************************************************************
函 数 名：  void CheckSceneDlyTimeEEPROM(void)
功能说明：  检查EEPROM中场景延时参数的合法性
参   数：   无
返 回 值：  无
*******************************************************************************/
void CheckSceneDlyTimeEEPROM(void)
{
	
#if 0
	u16 loopcnt,totalscene_num = 0x00;
	u8 unitCnt;
	u8 scenetime_flag = 0x00;
	u8 scenetimeuse_flag = 0x00;
	u8 scene_flag[UNIT_SCENEFLAG_NUM] = {0};
	u8 scenetimepos[SCENE_MAX_NUM] = {0};
#endif	


	
	u8 unitCnt;
	u16 loopcnt,totalscene_num = 0x00;
	u8 cnt;
	u16 i;
	u8 sceneAddr[SCENE_MAX_NUM];
	u8 scenetime_flag = 0x00;
//	u8 scenetimedata[SCENE_MAX_NUM];
	u8 scenetimeuse_flag = 0x00;
	u8 scenetimepos[SCENE_MAX_NUM] = {0};
	
	for (unitCnt = 1; unitCnt < UnitCnt_Get(); unitCnt++)
	{
		for(i=0;i<SCENE_MAX_NUM;i++)
		{
			if(unitCnt == gSceneBuff[i][0])
			{		
				sceneAddr[cnt++] = i+1;
			}
		}
	}
	
	
//	ReadDataFromEEPROM(SCENEFLAG_START_ADD + (UNIT_SCENEFLAG_SPACE * unitCnt),\
//                          sizeof(scene_flag), (u8 *)&scene_flag);
	for(loopcnt = 0;loopcnt < sizeof(sceneAddr);loopcnt ++)
	{
		if(SCENEFLAG_DEFAULT_VALUE != sceneAddr[loopcnt])			//检查当前单元场景是否存在
		{
			ReadDataFromEEPROM(SCENCETIMFLAG_START_ADD + sceneAddr[loopcnt],\
												sizeof(scenetime_flag), (u8 *)&scenetime_flag);
			if((SCENEFLAG_DEFAULT_VALUE != scenetime_flag)&&(scenetime_flag <= SCENE_TIMSPACE_NUM))	//判断场景时间映射表的参数是否合法
			{
				ReadDataFromEEPROM(SCENETIMUSEFLAG_START_ADD + ((scenetime_flag-1)/8),\
											sizeof(scenetimeuse_flag), &scenetimeuse_flag);
				if(scenetimeuse_flag&(1<<(scenetime_flag-1)%8))
				{
					scenetimepos[sceneAddr[loopcnt]] = scenetime_flag;
				}
				else
				{
					WriteDataToEEPROMEx(SCENCETIMFLAG_START_ADD + sceneAddr[loopcnt],\
												1, (u8)SCENEFLAG_DEFAULT_VALUE);
				}
			}
			continue;
		}	
		totalscene_num ++;
	}	
	
	
	if(sizeof(sceneAddr) == totalscene_num)	//若场景数据全部为空,则时间数据清空
	{
		WriteDataToEEPROMEx(SCENCETIMFLAG_START_ADD, SCENE_MAX_NUM, SCENEFLAG_DEFAULT_VALUE); 		//清除场景数据与时间数据的映射表,256bytes
		WriteDataToEEPROMEx(SCENETIMUSEFLAG_START_ADD, SCENE_MAX_NUM / 8, SCENEFLAG_DEFAULT_VALUE); //清除场景时间数据的分配表,32bytes
	}
	else
	{
		WriteDataToEEPROM(SCENCETIMFLAG_START_ADD, SCENE_MAX_NUM, scenetimepos);
		memset(sceneAddr,0,sizeof(sceneAddr));
		for(loopcnt = 0;loopcnt < SCENE_MAX_NUM;loopcnt ++)
		{
			if(SCENEFLAG_DEFAULT_VALUE != scenetimepos[loopcnt])
			{
				sceneAddr[(scenetimepos[loopcnt]-1)/8] |= (1 << ((scenetimepos[loopcnt]-1)%8));
			}				
		}
		WriteDataToEEPROM(SCENETIMUSEFLAG_START_ADD,SCENE_MAX_NUM/8,sceneAddr);
	}

	
#if 0	
    for (unitCnt = 1; unitCnt < UnitCnt_Get(); unitCnt++)
    {
      ReadDataFromEEPROM(SCENEFLAG_START_ADD + (UNIT_SCENEFLAG_SPACE * unitCnt),\
                          sizeof(scene_flag), (u8 *)&scene_flag);
			for(loopcnt = 0;loopcnt < sizeof(scene_flag);loopcnt ++)
			{
				if(SCENEFLAG_DEFAULT_VALUE != scene_flag[loopcnt])			//检查当前单元场景是否存在
				{
					ReadDataFromEEPROM(SCENCETIMFLAG_START_ADD + scene_flag[loopcnt],\
														sizeof(scenetime_flag), (u8 *)&scenetime_flag);
					if((SCENEFLAG_DEFAULT_VALUE != scenetime_flag)&&(scenetime_flag <= SCENE_TIMSPACE_NUM))	//判断场景时间映射表的参数是否合法
					{
						ReadDataFromEEPROM(SCENETIMUSEFLAG_START_ADD + ((scenetime_flag-1)/8),\
													sizeof(scenetimeuse_flag), &scenetimeuse_flag);
						if(scenetimeuse_flag&(1<<(scenetime_flag-1)%8))
						{
							scenetimepos[scene_flag[loopcnt]] = scenetime_flag;
						}
						else
						{
							WriteDataToEEPROMEx(SCENCETIMFLAG_START_ADD + scene_flag[loopcnt],\
														1, (u8)SCENEFLAG_DEFAULT_VALUE);
						}
					}
					continue;
				}	
				totalscene_num ++;
			}	
    }	
		///////////////////////
		
		
	if(unitCnt * UNIT_SCENEFLAG_NUM == totalscene_num)	//若场景数据全部为空,则时间数据清空
	{
		WriteDataToEEPROMEx(SCENCETIMFLAG_START_ADD, SCENE_MAX_NUM, SCENEFLAG_DEFAULT_VALUE); 		//清除场景数据与时间数据的映射表,256bytes
		WriteDataToEEPROMEx(SCENETIMUSEFLAG_START_ADD, SCENE_MAX_NUM / 8, SCENEFLAG_DEFAULT_VALUE); //清除场景时间数据的分配表,32bytes
	}
	else
	{
		WriteDataToEEPROM(SCENCETIMFLAG_START_ADD, SCENE_MAX_NUM, scenetimepos);
		memset(scene_flag,0,sizeof(scene_flag));
		for(loopcnt = 0;loopcnt < SCENE_MAX_NUM;loopcnt ++)
		{
			if(SCENEFLAG_DEFAULT_VALUE != scenetimepos[loopcnt])
			{
				scene_flag[(scenetimepos[loopcnt]-1)/8] |= (1 << ((scenetimepos[loopcnt]-1)%8));
			}				
		}
		WriteDataToEEPROM(SCENETIMUSEFLAG_START_ADD,SCENE_MAX_NUM/8,scene_flag);
	}
#endif
}

/*******************************************************************************
函 数 名：  SceneDlyTimeEEPROM_Set
功能说明：  写入一个场景的扩展接口,支持延时顺序执行
参   数：   unit:       单元位置
            area：      区域位置(0~4)
            num:        情景号
            cnt:        属性命令数量
            data[0~n]:  场景时间数据内容
返 回 值：  存储结果
*******************************************************************************/
static u32 SceneDlyTimeEEPROM_Set(u8 unit, u8 area, u8 num, u8 cnt, u8 *data)
{
	u8 scenetime_temp,
	scenetimeuseflag_temp[SCENE_TIMSPACE_NUM / 8],
	flag, i;
	
	
		if ((cnt > 0) && (cnt <= (SCENE_TIMDATA_SPACE / sizeof(DlyTimeData_t))))
    {
			flag = FindScene(unit,area,num);
			ReadDataFromEEPROM(SCENCETIMFLAG_START_ADD + flag, 1, &scenetime_temp);
			if ((!scenetime_temp)||(scenetime_temp > SCENE_TIMSPACE_NUM))         //没有申请时间存储内存则申请内存
			{
				ReadDataFromEEPROM(SCENETIMUSEFLAG_START_ADD, SCENE_TIMSPACE_NUM / 8, scenetimeuseflag_temp);
				for (i = 0, scenetime_temp = SCENEFLAG_DEFAULT_VALUE; i < SCENE_TIMSPACE_NUM; i++)
				{
					if (!(scenetimeuseflag_temp[i / 8] & (1 << (i % 8))))
					{
						scenetime_temp = i + 1;               //记录的场景时间存储标号从1~SCENE_TIMSPACE_NUM
						scenetimeuseflag_temp[i / 8] |= (1 << (i % 8));
						WriteDataToEEPROM(SCENETIMUSEFLAG_START_ADD + i / 8, 1, &scenetimeuseflag_temp[i / 8]);
						WriteDataToEEPROM(SCENCETIMFLAG_START_ADD + flag,1,&scenetime_temp);
						break;
					}
				}
				if (SCENEFLAG_DEFAULT_VALUE == scenetime_temp)
				{
					CheckSceneDlyTimeEEPROM();
					return EEPROM_RAND_ERR;
				}
			}
			WriteDataToEEPROM(SCENETIMSAVE_START_ADD + SCENE_TIMDATA_SPACE * (scenetime_temp - 1),\
												sizeof(DlyTimeData_t)*cnt, (u8 *)data);
			return COMPLETE;
    }
    else
    {
			return EEPROM_RAND_ERR;
    }
#if 0	
    if ((cnt > 0) && (cnt <= (SCENE_TIMDATA_SPACE / sizeof(DlyTimeData_t))))
    {
        ReadDataFromEEPROM(SCENEFLAG_START_ADD + (UNIT_SCENEFLAG_SPACE * unit) + (AREA_SCENEFLAG_SPACE * area) + (SCENEFLAG_SPACE * num),
                           sizeof(flag), (u8 *)&flag);
		ReadDataFromEEPROM(SCENCETIMFLAG_START_ADD + flag, 1, &scenetime_temp);
        if ((!scenetime_temp)||(scenetime_temp > SCENE_TIMSPACE_NUM))         //没有申请时间存储内存则申请内存
        {
            ReadDataFromEEPROM(SCENETIMUSEFLAG_START_ADD, SCENE_TIMSPACE_NUM / 8, scenetimeuseflag_temp);
            for (i = 0, scenetime_temp = SCENEFLAG_DEFAULT_VALUE; i < SCENE_TIMSPACE_NUM; i++)
            {
                if (!(scenetimeuseflag_temp[i / 8] & (1 << (i % 8))))
                {
                    scenetime_temp = i + 1;               //记录的场景时间存储标号从1~SCENE_TIMSPACE_NUM
                    scenetimeuseflag_temp[i / 8] |= (1 << (i % 8));
                    WriteDataToEEPROM(SCENETIMUSEFLAG_START_ADD + i / 8, 1, &scenetimeuseflag_temp[i / 8]);
					WriteDataToEEPROM(SCENCETIMFLAG_START_ADD + flag,1,&scenetime_temp);
                    break;
                }
            }
            if (SCENEFLAG_DEFAULT_VALUE == scenetime_temp)
            {
                CheckSceneDlyTimeEEPROM();
				return EEPROM_RAND_ERR;
            }
        }
        WriteDataToEEPROM(SCENETIMSAVE_START_ADD + SCENE_TIMDATA_SPACE * (scenetime_temp - 1),\
                          sizeof(DlyTimeData_t)*cnt, (u8 *)data);
		return COMPLETE;
    }
    else
    {
        return EEPROM_RAND_ERR;
    }
#endif
}


/*******************************************************************************
函 数 名：  SceneEEPROM_SetEx
功能说明：  写入一个场景的扩展接口,支持延时顺序执行
参   数：   unit:       单元位置
            area：      区域位置(0~4)
            num:        情景号
            data[0]:    场景内容长度
            data[1~n]:  场景内容
返 回 值：  存储结果
*******************************************************************************/
u32 SceneEEPROM_SetEx(u8 unit, u8 area, u8 num, u8 *data)
{
    u8 	data_temp1[SCENE_TIMDATA_SPACE],     //记录场景时间参数的缓冲数组
		data_temp2[SCENESAVE_DATA_SPACE],    //记录实际场景数据的缓冲数组
		len  = data[0];                      //记录原始数据的长度
    u8  pos  = 0,
        pos1 = 0,                            //记录缓冲数组场景时间偏移
        pos2 = 0,                            //记录缓冲数组场景实际数据偏移
        count = 0;                           //记录场景中执行属性的个数

    while (len > pos)                        //分离场景数据和时间数据，分别单独处理
    {
        memcpy(data_temp1 + pos1, &data[pos + 1] + ((UnitPara_t *)&data[pos + 1])->len + offsetof(UnitPara_t,data), sizeof(DlyTimeData_t));
        memcpy(data_temp2 + pos2, &data[pos + 1], ((UnitPara_t *)&data[pos + 1])->len + offsetof(UnitPara_t,data));
        pos1 += sizeof(DlyTimeData_t);
        pos2 += ((UnitPara_t *)&data[pos + 1])->len + offsetof(UnitPara_t,data);
        pos = pos1 + pos2;
        count ++;
    }
    data[0] = pos2;
    memcpy(data + 1, data_temp2, pos2);
	if(TRUE == CheckAllTimeParaIsOK((DlyTimeData_t*)&data_temp1,count))
	{
		if (COMPLETE == SceneEEPROM_Set(unit, area, num, data))
		{      
			if (COMPLETE == SceneDlyTimeEEPROM_Set(unit, area, num, count, data_temp1))
			{
				return COMPLETE;
			}
			SceneEEPROM_Clr(unit, area, num);
			return EEPROM_RAND_ERR;
		}
		return EEPROM_ERR;
	}
	return TIME_PARA_ERR;
}
#endif

/*******************************************************************************
函 数 名：  SceneEEPROM_Get
功能说明：  获取场景号标识是否有效
参   数：   unit:       单元位置
            area：      区域位置(0~4)
            num:        组号
            data[0]:    场景内容长度 data[0] = 0, 没有此场景
            data[1~n]:  场景内容
返 回 值：  标识结果 TRUE(有效)/FALSE(无效)
*******************************************************************************/
void SceneEEPROM_Get(u8 unit, u8 area, u8 num, u8 *data)
{
	u8 flag;
	flag = FindScene(unit,area,num);
	if(SCENEFLAG_DEFAULT_VALUE != flag)
	{
		ReadDataFromEEPROM(SceneEEPROMSave_Addr(flag),  //保存数据
												 SCENESAVE_DATA_SPACE, data);
	}
	else
	{
		*data = 0;  //返回场景长度为零
	}


#if 0
	ReadDataFromEEPROM(SCENEFLAG_START_ADD + (UNIT_SCENEFLAG_SPACE * unit) + (AREA_SCENEFLAG_SPACE * area) + (SCENEFLAG_SPACE * num),
										 sizeof(flag), (u8 *)&flag);
	if (flag != SCENEFLAG_DEFAULT_VALUE)    //场景存在
	{
			ReadDataFromEEPROM(SceneEEPROMSave_Addr(flag),  //保存数据
												 SCENESAVE_DATA_SPACE, data);
	}
	else
	{
			*data = 0;  //返回场景长度为零
	}
#endif
}

#if (SCENCETIME_EN > 0u)
/*******************************************************************************
函 数 名：  SceneEEPROM_GetEx
功能说明：  获取场景号标识是否有效
参   数：   unit:       单元位置
            area：     区域位置(0~4)
            num:        组号
            data[0]:    场景内容长度 data[0] = 0, 没有此场景
            data[1~n]:  场景内容
返 回 值：  标识结果 TRUE(有效)/FALSE(无效)
*******************************************************************************/
u8 SceneEEPROM_GetEx(u8 unit, u8 area, u8 num, u8 *data)
{
	u8 	flag = 0u;
	u8 	scenetime_temp = 0u,               			//记录场景时间参数的标示缓冲数组
	scence_temp1[SCENE_TIMDATA_SPACE],          //记录场景时间参数的缓冲数组
	data_temp[SCENE_TIMDATA_SPACE + SCENESAVE_DATA_SPACE];
	u8 	pos1 = 0u, pos2 = 0u, len = 0u;
	
#if 1
	
	flag = FindScene(unit,area,num);
	ReadDataFromEEPROM(SCENCETIMFLAG_START_ADD + flag, 1, &scenetime_temp);
	if (flag != SCENEFLAG_DEFAULT_VALUE)    			//场景存在
	{
			ReadDataFromEEPROM(SceneEEPROMSave_Addr(flag),  //保存数据
												 SCENESAVE_DATA_SPACE, data);
			len = data[0];
			if ((SCENEFLAG_DEFAULT_VALUE != scenetime_temp) && (scenetime_temp <= SCENE_TIMSPACE_NUM))
			{
					ReadDataFromEEPROM(SCENETIMSAVE_START_ADD + SCENE_TIMDATA_SPACE * (scenetime_temp - 1), //保存数据
														 SCENE_TIMDATA_SPACE, scence_temp1);
					while (len > pos1)
					{
							memcpy(data_temp + pos1 + pos2, &data[pos1 + 1], ((UnitPara_t *)&data[pos1 + 1])->len + offsetof(UnitPara_t,data));
							pos1 += ((UnitPara_t *)&data[pos1 + 1])->len + offsetof(UnitPara_t,data);
							memcpy(data_temp + pos1 + pos2, scence_temp1 + pos2, sizeof(DlyTimeData_t));
							pos2 += sizeof(DlyTimeData_t);
					}
					data[0] = pos1 + pos2;
					memcpy(&data[1], data_temp, pos1 + pos2);
		return TRUE;
			}
	}
	else
	{
			*data = 0;  //返回场景长度为零
	}
	
	
#endif
	
	
	
//////////////////////////////////////////////////	
#if 0
	ReadDataFromEEPROM(SCENEFLAG_START_ADD + (UNIT_SCENEFLAG_SPACE * unit) + (AREA_SCENEFLAG_SPACE * area) + (SCENEFLAG_SPACE * num),
										 sizeof(flag), (u8 *)&flag);
	ReadDataFromEEPROM(SCENCETIMFLAG_START_ADD + flag, 1, &scenetime_temp);

	if (flag != SCENEFLAG_DEFAULT_VALUE)    			//场景存在
	{
			ReadDataFromEEPROM(SceneEEPROMSave_Addr(flag),  //保存数据
												 SCENESAVE_DATA_SPACE, data);
			len = data[0];
			if ((SCENEFLAG_DEFAULT_VALUE != scenetime_temp) && (scenetime_temp <= SCENE_TIMSPACE_NUM))
			{
					ReadDataFromEEPROM(SCENETIMSAVE_START_ADD + SCENE_TIMDATA_SPACE * (scenetime_temp - 1), //保存数据
														 SCENE_TIMDATA_SPACE, scence_temp1);
					while (len > pos1)
					{
							memcpy(data_temp + pos1 + pos2, &data[pos1 + 1], ((UnitPara_t *)&data[pos1 + 1])->len + offsetof(UnitPara_t,data));
							pos1 += ((UnitPara_t *)&data[pos1 + 1])->len + offsetof(UnitPara_t,data);
							memcpy(data_temp + pos1 + pos2, scence_temp1 + pos2, sizeof(DlyTimeData_t));
							pos2 += sizeof(DlyTimeData_t);
					}
					data[0] = pos1 + pos2;
					memcpy(&data[1], data_temp, pos1 + pos2);
		return TRUE;
			}
	}
	else
	{
			*data = 0;  //返回场景长度为零
	}
#endif
/////////////////////////////////////////////////////////////////////
	
	return FALSE;
}
#endif

/*******************************************************************************
函 数 名：  SceneEEPROMSaveData_Clr
功能说明：  场景EEPROM存储数据清除
参   数：   number: 场景数据编号
返 回 值：  无
*******************************************************************************/
void SceneEEPROMData_Init(void)
{
	u8 i=0;
	for(i=0;i<32;i++)
	{
		gSceneIndex[i] = 0;
	}
  WriteDataToEEPROMEx(SCENESAVEFLAG_START_ADD , SCENE_MAX_NUM / 8, 0);
	WriteDataToEEPROMEx(SCENCETIMFLAG_START_ADD, SCENE_MAX_NUM, SCENEFLAG_DEFAULT_VALUE); 		//清除场景数据与时间数据的映射表,256bytes,jay add 2016.01.29 
	
#if (SCENCETIME_EN > 0u)
	WriteDataToEEPROMEx(SCENCETIMFLAG_START_ADD, SCENE_MAX_NUM, SCENEFLAG_DEFAULT_VALUE); 		//清除场景数据与时间数据的映射表,256bytes,jay add 2016.01.29 
    WriteDataToEEPROMEx(SCENETIMUSEFLAG_START_ADD, SCENE_MAX_NUM / 8, SCENEFLAG_DEFAULT_VALUE); //清除场景时间数据的分配表,32bytes,jay add 2016.01.29
#endif
}

///*******************************************************************************
//函 数 名：  SceneEEPROMSaveData_Clr
//功能说明：  场景EEPROM存储数据清除
//参   数：   number: 场景数据编号
//返 回 值：  无
//*******************************************************************************/
//void SceneEEPROMData_Clr(u8 number)
//{
////	u8 i;
//	gSceneIndex[(number-1)/8] &= ~(1<<((number-1)%8));
//	WriteDataToEEPROM(SCENESAVEFLAG_START_ADD + (number-1) / 8, 1, &gSceneIndex[(number-1)/8]);
//	
//	
//#if 0
//	ReadDataFromEEPROM(SCENESAVEFLAG_START_ADD + number / 8, 1, &i);
//	i &= ~(1 << number % 8);
//	WriteDataToEEPROM(SCENESAVEFLAG_START_ADD + number / 8, 1, &i);
//#endif
//}

#if (SCENCETIME_EN > 0u)
/*******************************************************************************
函 数 名：  SceneEEPROMSaveData_ClrEx
功能说明：  场景EEPROM存储的时间数据清除
参   数：   number: 场景数据编号(0x01~0xff)
返 回 值：  无
*******************************************************************************/
static void SceneEEPROMData_ClrEx(u8 number)
{
    u8 i = 0u;
	if(number < SCENE_TIMSPACE_NUM)
	{
		ReadDataFromEEPROM(SCENETIMUSEFLAG_START_ADD + number / 8, 1, &i);
		i &= ~(1 << number % 8);
		WriteDataToEEPROM(SCENETIMUSEFLAG_START_ADD + number / 8, 1, &i);
	}
}

/*******************************************************************************
函 数 名：  static void  SceneEEPROMTimeFlag_Clr(u8 label)
功能说明：  场景EEPROM存储的时间数据标号和内存使用标记清除
参   数：   label: 场景数据编号(0x01~0xff)
返 回 值：  无
*******************************************************************************/
static void  SceneEEPROMTimeFlag_Clr(u8 label)
{
	u8 scenetime_temp = 0u;
	ReadDataFromEEPROM(SCENCETIMFLAG_START_ADD + label, 1, &scenetime_temp);
	if(scenetime_temp > 0u)
	{
		SceneEEPROMData_ClrEx(scenetime_temp - 1);						//清除场景时间数据内存块的使用标记					
		WriteDataToEEPROMEx(SCENCETIMFLAG_START_ADD + label,1, SCENEFLAG_DEFAULT_VALUE);
	}
}
#endif 

/*******************************************************************************
函 数 名：  SceneEEPROMSaveData_Seat
功能说明：  申请一个场景存储位置
参   数：   len:    目标存储数据长度
            number: 存储的位置值
返 回 值：  无
*******************************************************************************/
void SceneEEPROMData_Seat(u8 len, u8 *number)
{
//    u8 seat[SCENE_MAX_NUM / 8];
    u32 i;

    ReadDataFromEEPROM(SCENESAVEFLAG_START_ADD, sizeof(gSceneIndex), gSceneIndex);    //获取存储标识

    for (i = SceneEEPROMSave_Seat(len), *number = SCENEFLAG_DEFAULT_VALUE; i < SCENE_MAX_NUM; i++)
    {
        if (!(gSceneIndex[(i-1) / 8] & (1 << ((i-1) % 8)))) //如果存储位置是空闲的
        {
            *number = i;            //存储位置确定
            gSceneIndex[(i-1) / 8] |= (1 << ((i-1) % 8));
            WriteDataToEEPROM(SCENESAVEFLAG_START_ADD + (i-1) / 8, 1, &gSceneIndex[(i-1) / 8]); //修改存储标识·
            break;
        }
    }
}

/*******************************************************************************
函 数 名：  SceneEEPROMData_Save
功能说明：  场景EEPROM存储数据
参   数：   addr:   存储空地址
            data:   要存储的场景数据
返 回 值：  存储编号（0～127）/0xFF(没有存储空间)
*******************************************************************************/
void SceneEEPROMData_Save(u32 addr, u8 *data)
{
    WriteDataToEEPROM(addr, data[0] + 1, data); //保存数据
}

/*******************************************************************************
函 数 名：  SceneEEPROMSave_Seat
功能说明：     场景EEPROM存储位置
参   数：    seat:   存储长度
返 回 值：  存储位置
*******************************************************************************/
u32 SceneEEPROMSave_Seat(u8 len)
{
    if (len < SCENE_DATA_SPACE1)    //长度处于第一存储空间
    {
        return  1;
    }
    else if (len < (SCENE_DATA_SPACE2))//长度处于第二存储空间
    {
        return SCENE_SPACE1_NUM;
    }
    else if (len < (SCENE_DATA_SPACE3))//长度处于第三存储空间
    {
        return (SCENE_SPACE1_NUM + SCENE_SPACE2_NUM);
    }

    return SCENE_MAX_NUM;   //长度越界
}

/*******************************************************************************
函 数 名：  SceneEEPROMSave_Addr
功能说明：  场景EEPROM存储地址
参   数：   seat:   存储编号
返 回 值：  存储地址
*******************************************************************************/
u32 SceneEEPROMSave_Addr(u8 seat)
{
    u32 addr = SCENESAVE_START_ADD;

    if (seat < SCENE_SPACE1_NUM)
    {
        addr += seat * SCENE_DATA_SPACE1;
        goto RETURN;//位置处于存储空间1
    }

    addr += SCENE_SPACE1_NUM * SCENE_DATA_SPACE1;
    seat -= SCENE_SPACE1_NUM;
    if (seat < SCENE_SPACE2_NUM)
    {
        addr += seat * SCENE_DATA_SPACE2;
        goto RETURN;//位置处于存储空间2
    }

    addr += SCENE_SPACE2_NUM * SCENE_DATA_SPACE2;
    seat -= SCENE_SPACE2_NUM;
    if (seat < SCENE_SPACE3_NUM)
    {
        addr += seat * SCENE_DATA_SPACE3;
        goto RETURN;//位置处于存储空间3
    }

RETURN:
    return addr;
}

/*******************************************************************************
函 数 名：  EEPROM_Write
功能说明：  EEPROM_写入数据
参   数：   参数:  addr-EEPROM中的实际地址
            len-长度
            data-要写入的相同数据
  返回值:   消息执行结果
*******************************************************************************/
MsgResult_t EEPROM_Write(u32 addr, u32 len, u8 *data)
{
    if ((addr + len) > A24C256          //地址越界
            || addr < USERDATA_END_ADD
       )
    {
        return EEPROM_RAND_ERR;
    }

    if (WriteDataToEEPROM(addr, len, data) == FALSE)
    {
        return EEPROM_ERR;
    }

    return COMPLETE;
}
/*******************************************************************************
函 数 名：  EEPROM_Read
功能说明：  EEPROM读取数据
参   数：   参数:  addr-EEPROM中的实际地址
            len-长度
            data-要写入的相同数据
  返回值:   消息执行结果
*******************************************************************************/
MsgResult_t EEPROM_Read(u32 addr, u32 len, u8 *data)
{
    if ((addr + len) > A24C256          //地址越界
            || addr < USERDATA_END_ADD
       )
    {
        return EEPROM_RAND_ERR;
    }

    if (ReadDataFromEEPROM(addr, len, data) == FALSE)
    {
        return EEPROM_ERR;
    }

    return COMPLETE;
}

/**************************Copyright BestFu 2014-05-14*************************/
