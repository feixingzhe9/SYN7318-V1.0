/***************************Copyright BestFu 2014-05-14*************************
文	件：	Updata.c
说	明：	升级数据处理函数
编	译：	Keil uVision4 V4.54.0.0
版	本：	v1.0
编	写：	Unarty
日	期：	2013-11-24
修　改：	2015.01.14 Unarty 添加程序空间核对，防止空间越界导致设备升级失败或死机
*******************************************************************************/
#include "Updata.h"
#include "crc.h"
#include "program.h"
#include "BF_type.h"

Soft_t IAP_FLAG;    //IAP标志
RadioPara_t RADIO_PARA ; //433通信参数
u8 PACK_SIZE;			//包有效字节长度
u32 PACK_ID[64];	//512个包

/*******************************************************************************
函 数 名：  Updata_Check
功能说明：  更新核对
参	  数：  refer:	参考条件
			start: 	起始条件
			end:	结束条件
			
返 回 值：  TRUE/FALSE
*******************************************************************************/
u8 Updata_Check(u32 refer, u32 *start, u32 *end)
{
	if (refer < *start || refer > *end)
	{
		return FALSE;
	}

	return TRUE;
}
	
/*******************************************************************************
函 数 名：  Updata_ready
功能说明：  升级准备
参	  数：  size:	每个升级包的数据大小
			flag:	升级标识内容
返 回 值：  准备结果 TRUE(完成）/FALSE(失败）
*******************************************************************************/
u8 Updata_Ready(u8 size, Soft_t *flag)
{
    u8 i;
	u32 userSize;	//程序空间大小 Unarty 2015.01.04 Add 防止更新程序空间越界
	
	if (size&0x03)	//如果包的大小不是按4字节对齐
	{
		return FALSE;
	}
	switch (*(u16*)USER_FLASH_AREA)	//获取可用空间大小
	{
		case 64:			//64K
			userSize =  (31*0x400);
			break;
		case 128:			//128k
			userSize =  (47*0x400);
			break;
		case 256:			//256k
			userSize =  (100*0x400);
			break;
		case 512:			//512K
			userSize =  (200*0x400);
			break;
		default : 
			userSize = 0;
			break;	
	}
	if ((size*flag->appSize) > userSize)	//如果应用空间大于可用空间大小
	{
		return FALSE;
	}
	
	PACK_SIZE = size;
	IAP_FLAG.name = flag->name;
	IAP_FLAG.version = flag->version;
	IAP_FLAG.appAddress = flag->appAddress;
	IAP_FLAG.appSize = flag->appSize;
	
	for (i = 0; i < sizeof(PACK_ID)/sizeof(PACK_ID[0]); i++)	//数据包接收标识ID置1
	{
		PACK_ID[i] = 0;  
	}
	
	return Program_Erase(IAP_FLAG.appAddress, userSize);  //对应存储空间清零
}

/******************************************************************************* 
函 数 名：  Updata_PackProcess
功能说明：  升级数据包处理
参	  数：  id:		对应包的ID
			len:	包内数据长度
			data:	数据内容
返 回 值：  FALSE/TRUE
*******************************************************************************/
void Updata_PackProcess(u32 id, u8 len, u8 *data)
{
    u32 addr;

   	if (id > IAP_FLAG.appSize || len > 200)
	{
		return ;
	}
	if (!(PACK_ID[id>>5]&(1<<(id&0x1f))))
	{
	    addr = id*PACK_SIZE + IAP_FLAG.appAddress;
	    if (Program_Write(addr, len, data))
	    {
	        PACK_ID[id>>5] |= (1<<(id&0x1f)); //接收包ID置1
	    }
	}
}

/*******************************************************************************
函 数 名：	Updata_PackCheck
功能说明： 	升级数据包核对
参	  数： 	*len：	缺包数据长度
			*data:  缺包ID
返 回 值：	TRUE/FLASE
*******************************************************************************/
u8 Updata_PackCheck(u8 *num, u16 *data)
{
    u32 i;

    for (i = 0, *num = 0; i < IAP_FLAG.appSize; i++)
	{
        if (!(PACK_ID[i>>5]&(1<<(i&0x1f))))//包未收到
        {
			*data++ = i;
			(*num) += 2;
			if ((*num) > 100)
			{
				break;
			}
			
        }
    }

    return (*num == 0 ? TRUE : FALSE);
}

/*******************************************************************************
函 数 名：	Updata_Calibrate
功能说明： 	设备升级结果校验
参	  数： 	len:	数据长度
			crc:	校验码
返 回 值：	TRUE/FLASE
*******************************************************************************/
u8 Updata_Calibrate(u32 len, u16 crc)
{   
    crc -= ChkCrcValue((u8*)IAP_FLAG.appAddress, len);

    return (crc ? FALSE : TRUE);
}

/*******************************************************************************
函 数 名：	Updata_Flag
功能说明： 	升级标识
参	  数： 	add:	标识区存储地址
返 回 值：	TRUE/FLASE
*******************************************************************************/
u8 Updata_Flag(u32 add)
{
	/*Unarty Add 2014.06.09 433 ch 改变*/
	u8 ch = Channel_Get();
	
	Program_Erase(add, PAGE_SIZE);	//修改Flag区域
	Program_Write(add, sizeof(IAP_FLAG), (u8*)&IAP_FLAG);
	
	return Updata_Channel(ch);
}

/*******************************************************************************
函 数 名：	Updata_Channel
功能说明： 	更新433通道
参	  数： 	ch:		修改后的信道值
返 回 值：	TRUE/FLASE
*******************************************************************************/
u8 Updata_Channel(u8 ch)
{
	u32 *radioParaAddr;
	RADIO_PARA.channel = ch ;
	
	for (radioParaAddr = (u32*)RADIOPARA_ADD; 
			(u32)radioParaAddr < (FLAGADDR + PAGE_SIZE); radioParaAddr++)
	{
		if (*radioParaAddr == Flash_DefaultValue())		//数据没有修改
		{			
			return Program_Write((u32)radioParaAddr, sizeof(radioParaAddr), (u8*)&RADIO_PARA); //返回修改结果
		}
		RADIO_PARA.encrypt = (*radioParaAddr&0x00FF0000)>>16 ;
		RADIO_PARA.pace = (*radioParaAddr&0xFF000000)>>24 ;
	}
	Program_Read(FLAGADDR, sizeof(IAP_FLAG), (u8*)&IAP_FLAG);
	Program_Erase(FLAGADDR, PAGE_SIZE);	//清空数据	
	Program_Write(FLAGADDR, sizeof(IAP_FLAG), (u8*)&IAP_FLAG);	//数据写入
	
	return Updata_Channel(ch);
}

/*******************************************************************************
函 数 名：	Channel_Get
功能说明： 	获取433通道值 
参	  数： 	无
返 回 值：	433通道值
*******************************************************************************/
u8 Channel_Get(void)
{
	u32 *radioParaAddr; // 433通信参数存储地址
	
	for (radioParaAddr = (u32*)RADIOPARA_ADD; 
			(u32)radioParaAddr < (FLAGADDR + PAGE_SIZE); radioParaAddr++)
	{
		if (*radioParaAddr == Flash_DefaultValue())		//数据没有修改
		{
			radioParaAddr--;		//指针指向有效空间
			if (((*radioParaAddr&0x0000FFFF) < CHANNEL_MIN)
				|| ((*radioParaAddr&0x0000FFFF) > CHANNEL_MAX)
				)
			{
				break;
			}	
			return (*radioParaAddr&0x0000FFFF);	//返回最后通道设置值
		}
	}
	
	return 0x53;		//返回默认通道值
}
/*******************************************************************************
函 数 名：	Updata_433EncryptFlag
功能说明： 	更新433通信加密标记
参	  数： 	ch:		修改后的信道值
返 回 值：	TRUE/FLASE
*******************************************************************************/
u8 Updata_433EncryptFlag(u8 flag)
{
	u32 *radioParaAddr;
	RADIO_PARA.encrypt = 0x4B*flag+0x5A ; //将0/1映射为0x5A/0xA5
	
	for (radioParaAddr = (u32*)RADIOPARA_ADD; 
			(u32)radioParaAddr < (FLAGADDR + PAGE_SIZE); radioParaAddr++)
	{
		if (*radioParaAddr == Flash_DefaultValue())		//数据没有修改
		{			
			return Program_Write((u32)radioParaAddr, sizeof(radioParaAddr), (u8*)&RADIO_PARA); //返回修改结果
		}
		RADIO_PARA.channel = *radioParaAddr&0x0000FFFF ;
		RADIO_PARA.pace = (*radioParaAddr&0xFF000000)>>24 ;
	}
	Program_Read(FLAGADDR, sizeof(IAP_FLAG), (u8*)&IAP_FLAG);
	Program_Erase(FLAGADDR, PAGE_SIZE);	//清空数据	
	Program_Write(FLAGADDR, sizeof(IAP_FLAG), (u8*)&IAP_FLAG);	//数据写入
	
	return Updata_433EncryptFlag(flag);
}

/*******************************************************************************
函 数 名：	Get_433EncryptFlag
功能说明： 	获取433通信加密标记 
参	  数： 	无
返 回 值：	标记值
注    意：  上电先执行该函数，因此校验使用标记位的值
*******************************************************************************/
u8 Get_433EncryptFlag(void)
{
	u32 *radioParaAddr; // 433通信参数存储地址
	
	for (radioParaAddr = (u32*)RADIOPARA_ADD; 
			(u32)radioParaAddr < (FLAGADDR + PAGE_SIZE); radioParaAddr++)
	{
		if (*radioParaAddr == Flash_DefaultValue())		//数据没有修改
		{
			radioParaAddr--;		//指针指向有效空间
			if (((u8)((*radioParaAddr&0x00FF0000)>>16) != ENCRYPT)
				&& ((u8)((*radioParaAddr&0x00FF0000)>>16) != DECRYPT)
				)
			{
				break;
			}	
			return ((*radioParaAddr&0x00FF0000)>>16)&0x01;	//返回最后通道偏移设置值
		}
	}	
	return 0x01;		//返回默认加密标记值 通信加密
}
/**************************Copyright BestFu 2014-05-14*************************/
