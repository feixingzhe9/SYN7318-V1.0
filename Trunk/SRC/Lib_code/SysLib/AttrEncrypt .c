/***************************Copyright BestFu 2014-05-14*************************
文	件：	AttrEncrypt.c
说	明：	属性加密原文件
编	译：	Keil uVision4 V4.54.0.0
版	本：	v1.0
编	写：	Unarty
日	期：	2014-01-26
修　改：	无
*******************************************************************************/
#include "AttrEncrypt.h"
#include "MsgPackage.h"
#include "UnitCfg.h"
#include "crc.h"
#include "Thread.h"
#include "SysTick.h"

#define CMD_ADD_FLAG	(0xAA5AA5AA)	//指令加密标记
#define CMD_SUB_FLAG	(0xAAA55AAA)	//指令解密标记
#define ENCRYPT_NUM		(sizeof(Encrypt)/sizeof(Encrypt_t))	//加密数

static u8 EncryptFlag = 0;		//加密标识

//static Encrypt_t Encrypt[5] = {0};					//执行数

//static u8 ExeSrc_Check(u32 srcAddr);
//static u8 ExeSrc_Add(Communication_t *cmd);
//static void ExeSrc_TimeCheck(void);
static void Encrypt_FlagClr(void);
static void Encrypt_Data(u8 *data, u16 key);


/*******************************************************************************
函 数 名：  AttrEncrypt_Check
功能说明：  属性加密核对
参	  数：  cmd:	目标执行指令			
返 回 值：  TRUE(核对成功)/FALSE
*******************************************************************************/
u8 AttrEncrypt_Check(Communication_t *cmd)
{	
	u16 i;
	
//	if (*(u32*)(&cmd->para.len + cmd->para.len - 3) == CMD_SUB_FLAG)//密钥指令回应
//	{
//		if ((i = ExeSrc_Check(cmd->aim.object.id)) < ENCRYPT_NUM)		//有数据源
//		{
//			cmd->para.len -= 4;		//去除指令标识
//			Encrypt_Data(&cmd->para.len, Encrypt[i].prd);	//公钥解密
//			Encrypt_Data(&cmd->para.len, Encrypt[i].prv);	//私钥解密
//			if (Encrypt[i].crc == ChkCrcValue(&cmd->para.len + 1, cmd->para.len)) //数据解码成功
//			{
//				cmd->aim.serialNum--;
//				return TRUE;
//			}
//		}
//	}
//	else if (*(u32*)(&cmd->para.len + cmd->para.len - 3) == CMD_ADD_FLAG)//指令加密
	if (*(u32*)(&cmd->para.len + cmd->para.len - 3) == CMD_ADD_FLAG)//指令加密
	{
		if (EncryptFlag)	// 如果自己发送了指令
		{
			Encrypt_FlagClr();
			i = (*(u16*)(&cmd->para.len + cmd->para.len - 5));	//获取公钥
			cmd->para.len -= 6;		//减去指令标识与公钥
			Encrypt_Data(&cmd->para.len, i);	//公钥加密数据
			*(u32*)(&cmd->para.len + cmd->para.len + 1) = CMD_SUB_FLAG; //添加指令标识
			cmd->para.len += 4;
			return TRUE;
		}
	}
//	else
//	{
//		if (cmd->aim.objectType == SINGLE_ACT		//添加一个执行管理
//			&& TRUE == ExeSrc_Add(cmd))	//目标操作为单点操作
//		{
//			*(u32*)(&cmd->para.len + cmd->para.len + 1) = CMD_ADD_FLAG;	//参数尾添加密钥指令标记
//			cmd->para.len += 4;
//			cmd->aim.serialNum++;
//			Msg_Feedback(cmd);	//将消息返回给源设备
//		}
//	}

	return FALSE;
}

/******************************************************************************* 
函 数 名：  Encrypt_FlagSet
功能说明：  密钥设置
参	  数：  无
返 回 值：  无
注	  意：	函数将修改 变量EncryptFlag 。并在3s后自动调用Encrypt_FlagClr清除变量
*******************************************************************************/	
void Encrypt_FlagSet(void)
{
	EncryptFlag = 1;
	Thread_Login(ONCEDELAY, 0, 3000, Encrypt_FlagClr);
}

/******************************************************************************* 
函 数 名：  Encrypt_FlagClr
功能说明：  密钥清除
参	  数：  无
返 回 值：  无
注	  意：	函数将修改 变量EncryptFlag
*******************************************************************************/	
static void Encrypt_FlagClr(void)
{
	EncryptFlag = 0;
}

/******************************************************************************* 
函 数 名：  Encrypt_Data
功能说明：  加密数据
参	  数：  data:	data[0]数据长度, data[1~data[0]] 数据内容
			key:	钥匙
返 回 值：  无
*******************************************************************************/	
static void Encrypt_Data(u8 *data, u16 key)
{
	u8 i;
	
	for (i = 0; i < data[0]; i++)
	{
		i++;
		*(u16*)&data[i] ^= key;
	}
}
	
///*******************************************************************************
//函 数 名：  ExeSrc_Check
//功能说明：  核对指令源地址
//参	  数：  源地址
//返 回 值：  0(没有查找到数据）/非零：数据的存储位置
//*******************************************************************************/
//static u8 ExeSrc_Check(u32 srcAddr)
//{
//    u8 i;
//	
//	for (i = 0; i < ENCRYPT_NUM; i++)
//	{
//		if (Encrypt[i].srcAddr == srcAddr)
//		{
//			break;
//		}
//	}
//	
//	return i;
//}

///******************************************************************************* 
//函 数 名：  ExeSrc_Add
//功能说明：  添加指令执行源
//参	  数：  srcAddr:	源地址
//返 回 值：  FALSE/TRUE
//*******************************************************************************/
//static u8 ExeSrc_Add(Communication_t *cmd)
//{
//	u8 i;
//	
//	if ((i = ExeSrc_Check(cmd->aim.object.id)) >= ENCRYPT_NUM)	//如果列表中忆存在此联动
//	{
//		for (i = 0; i < ENCRYPT_NUM; i++)
//		{
//			if (0 == Encrypt[i].srcAddr)
//			{
//				Encrypt[i].cnt = 1;
//				break;
//			}
//		}
//	}
//	else
//	{
//		Encrypt[i].cnt++;
//	}
//	
//	if (Encrypt[i].cnt > 5)
//	{
//		//上报异常
//	}
//	else
//	{
//		Encrypt[i].time = 5;
//		Encrypt[i].srcAddr = cmd->aim.object.id;
//		Encrypt[i].crc = ChkCrcValue(&cmd->para.len + 1, cmd->para.len);
//		Encrypt[i].prd = cmd->aim.serialNum + cmd->aim.sourceID 	//生成随机公钥
//						+ cmd->aim.object.area[cmd->aim.serialNum%4] + Time_Get();
//		Encrypt[i].prv = *(u16*)(Encrypt[i].prd%0xF000 + 0x08000000);	//到Flash中随机获取一个私钥
//		
//		Encrypt_Data(&cmd->para.len, Encrypt[i].prv);	//私钥加密数据
//		*(u16*)(&cmd->para.len + cmd->para.len + 1) = Encrypt[i].prd;	//公钥放入参数区
//		cmd->para.len += sizeof(Encrypt[i].prd);
//		
//		Thread_Login(FOREVER, 0, 1000, ExeSrc_TimeCheck);
//		
//		return TRUE;
//	}
//	return FALSE;
//}

///******************************************************************************* 
//函 数 名：  ExeSrc_TimeCheck
//功能说明：  执行源时间核对
//参	  数：  无
//返 回 值：  无
//*******************************************************************************/
//static void ExeSrc_TimeCheck(void)
//{
//	u8 i, j;
//	
//	for (i = 0, j = 0; i < ENCRYPT_NUM; i++)
//	{
//		if (Encrypt[i].srcAddr != 0)
//		{
//			if (0 == (--Encrypt[i].time))
//			{
//				Encrypt[i].srcAddr = 0;
//			}
//			else
//			{
//				j++;
//			}
//		}
//	}
//	
//	if (0 == j)
//	{
//		Thread_Logout(ExeSrc_TimeCheck);
//	}
//}	

/**************************Copyright BestFu 2014-05-14*************************/
