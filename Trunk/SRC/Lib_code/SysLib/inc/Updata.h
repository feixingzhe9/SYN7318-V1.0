/***************************Copyright BestFu 2014-05-14*************************
文	件：	Updata.h
说	明：	升级数据处理函数
编	译：	Keil uVision4 V4.54.0.0
版	本：	v1.0
编	写：	Unarty
日	期：	2013-11-24
修　改：	暂无
*******************************************************************************/
#ifndef __UPDATA_H
#define __UPDATA_H

#include "BF_type.h"

#define HAERADDR	(0x08000300)	//硬件版本地址

//#define IAPADDR	(0x08000000)	//IAP区域区域首地址 size(8K)
#define FLAGADDR	(0x08000400)	//标志存储区域区域首地址 size(1K)
#define APPADDR_A   (0x08000800)	//应用程序存储区域首地址 size(27K)
//#define APPADDR_B	(0x08008400)	//转存数据存储区域首地址 size(28K)

#ifdef 	STM32L151
	#define USER_FLASH_AREA			(0X1FF8004C)
#else
	#define USER_FLASH_AREA			(0x1FFFF7E0)	//F103
#endif

#define CHANNEL_MIN				(73)
#define CHANNEL_MAX				(93)

#define PACE_10M                (0xA5)
#define PACE_1M                 (0x5A)
#define ENCRYPT                 (0xA5)
#define DECRYPT                 (0x5A)
#define RADIOPARA_ADD			(0x08000420) //433通信参数存储地址

typedef struct
{
	u32 hardVer;
	u32 Reserve1;
	u32 data;
	u32 time;
	u8  str[100];
}Hard_t;

typedef  struct
{
    u32 name;//设备名称
    u32	version;//软件版本号
	u32 appAddress;//APP程序的起始地址
    u32	appSize;//：APP程序的大小，以字节为单位  
}Soft_t;

#pragma pack(1)
typedef  struct
{
    u16 channel; //信道
    u8	encrypt; //通信加密
	u8  pace;    //信道间隔
}RadioPara_t;
#pragma pack()

#define HARD				((Hard_t*)HAERADDR)
#define SOFT			 	((Soft_t*)FLAGADDR)


u8 Updata_Check(u32 refer, u32 *start, u32 *end);
u8 Updata_Ready(u8 size, Soft_t *falg);
void Updata_PackProcess(u32 id, u8 len, u8 *data);
u8 Updata_PackCheck(u8 *num, u16 *data);
u8 Updata_Calibrate(u32 len, u16 crc);
u8 Updata_Flag(u32 add);
u8 Updata_Channel(u8 ch);
u8 Channel_Get(void);
u8 Updata_433EncryptFlag(u8 flag);
u8 Get_433EncryptFlag(void);

#endif  //Updata.h  end
/**************************Copyright BestFu 2014-05-14*************************/
