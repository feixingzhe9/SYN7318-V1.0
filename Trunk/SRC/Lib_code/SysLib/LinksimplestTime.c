/***************************Copyright BestFu 2014-05-14*************************
文	件：	LinksimplestTime.c
说	明：	最简联动带延时源文件
编	译：	Keil uVision4 V5.12.0.0
版	本：	v1.0
编	写：	Joey
日	期：	2016.1.29 
修　改：	
*******************************************************************************/

#include "Linkall.h"

#if (ALINKTIME_EN > 0)
ALinkTimeTable   ALinkTime;   //最简联动的RAM映射变量

/*******************************************************************************
函 数 名：	ALinkTimeInit
功能说明： 	最简联动表延时功能的初始化
参	  数： 	void
返 回 值：	void
*******************************************************************************/
void ALinkTimeInit(void)
{
	if (FALSE == ReadDataFromEEPROM(GetALinkTimeStartAddr(), sizeof(ALinkTimeTable), 
							(u8 *)&ALinkTime))
	{
		return;
	}
	if(ALinkTime.Cnt > MAX_ALINKTIMEITEM)
	{
		AllALinkTimeClr();
	}
}

/*******************************************************************************
函 数 名：	AllALinkTimeClr
功能说明： 	清除最简联动表的延时时间
参	  数： 	void
返 回 值：	void
*******************************************************************************/
void AllALinkTimeClr(void)
{
	if(ALinkTime.Cnt != 0)
	{
		memset((u8 *)&ALinkTime, 0, sizeof(ALinkTimeTable));
		WriteDataToEEPROM(GetALinkTimeStartAddr(), sizeof(ALinkTimeTable), (u8 *)&ALinkTime);
	}
}

/*******************************************************************************
函 数 名：	QueryALinkTime
功能说明： 	查询num联动号在延时时间表中的位置
参	  数： 	num-要查询的联动号
返 回 值：	EXEERROR表示没找到，0-254表示找到的实际位置
*******************************************************************************/
u8 QueryALinkTime(u8 num)
{
	u8 i;
	for(i=0;i<ALinkTime.Cnt;i++)
	{
		if(ALinkTime.Item[i].UseFlag)
		{
			if(ALinkTime.Item[i].LinkNum == num)
			{
				return i;
			}
		}
	}
	return EXEERROR;
}

/*******************************************************************************
函 数 名：	DelALinkTime
功能说明： 	把延时时间从延时时间表中删除
参	  数： 	num-要删除的联动位置
返 回 值：	1表示删除成功，0表示删除失败
*******************************************************************************/
u8 DelALinkTime(u8 num)
{
	if(num != (ALinkTime.Cnt-1))
	{
		memcpy((u8 *)&ALinkTime.Item[num],
			(u8 *)&ALinkTime.Item[ALinkTime.Cnt-1],sizeof(ALinkTimeItem));
		WriteDataToEEPROM(GetALinkTimeStartAddr() + sizeof(ALinkTimeItem)*num, 
				sizeof(ALinkTimeItem), (u8 *)&ALinkTime.Item[num]);
		
		memset((u8 *)&ALinkTime.Item[ALinkTime.Cnt-1], 0, sizeof(ALinkTimeItem));
		WriteDataToEEPROM(GetALinkTimeStartAddr() + sizeof(ALinkTimeItem)*(ALinkTime.Cnt-1), 
				sizeof(ALinkTimeItem), (u8 *)&ALinkTime.Item[num]);
	}
	else
	{
		memset((u8 *)&ALinkTime.Item[num], 0, sizeof(ALinkTimeItem));
		WriteDataToEEPROM(GetALinkTimeStartAddr() + sizeof(ALinkTimeItem)*num, 
				sizeof(ALinkTimeItem), (u8 *)&ALinkTime.Item[num]);
	}
	
	if (ALinkTime.Cnt > 0)
	{
		ALinkTime.Cnt--;
		WriteDataToEEPROM(GetALinkTimeStartAddr() + ((u32)&ALinkTime.Cnt - (u32)&ALinkTime), 
				sizeof(u8), (u8 *)&ALinkTime.Cnt);
		return TRUE;
	}

	return FALSE;
}

/*******************************************************************************
函 数 名：	WriteALinkTime
功能说明： 	把延时时间写入延时时间表中
参	  数： 	num-要写入的位置
						linknum-联动号
            addflag-表示新增
            *data-要写入的数据
返 回 值：	1表示写入成功，0表示写入失败
*******************************************************************************/
u8 WriteALinkTime(u8 num, u8 linknum, u8 addflag, u8 *data)
{
	if(num <= MAX_ALINKTIMEITEM)
	{
		memcpy(&ALinkTime.Item[num].Time.delaytime[0], data, sizeof(DlyTimeData_t));
		ALinkTime.Item[num].UseFlag = 1;
		ALinkTime.Item[num].LinkNum = linknum;
		
		WriteDataToEEPROM(GetALinkTimeStartAddr() + sizeof(ALinkTimeItem)*num, 
				sizeof(ALinkTimeItem), (u8 *)&ALinkTime.Item[num]);

		if(addflag)
		{
			ALinkTime.Cnt++;
			WriteDataToEEPROM(GetALinkTimeStartAddr() + ((u32)&ALinkTime.Cnt - (u32)&ALinkTime), 
							sizeof(u8), &ALinkTime.Cnt);
		}
		return TRUE;
	}
	return FALSE;
}

/*******************************************************************************
函 数 名：	AddALinkTimeCmdX1
功能说明： 	核实是否有延时执行的动作，如果有拼成0x91指令，往外发送
参	  数： 	num-表示位置
            *data-要拼成0X91指令的联动数据
返 回 值：	1表示读取成功，0表示读取失败
*******************************************************************************/
u8 AddALinkTimeCmdX1(u8 num, u8 *data)
{
	u8 cnt,time[5]={0};
	ALinkItem *pLink;
	pLink = (ALinkItem *)data;
	
	memmove(&pLink->data[0], &pLink->UnitID, pLink->DestLen + 3);
	
	ReadALinkTime(num , &cnt, &time[0]);
	memcpy(&pLink->data[pLink->DestLen + 3], &time[0], sizeof(DlyTimeData_t));
	
	pLink->UnitID  = 0;
	pLink->DestCmd = 0x91;
	pLink->DestLen += 3;
	return TRUE;
}

/*******************************************************************************
函 数 名：	ReadALinkTime
功能说明： 	从最简联动表中读取联动
参	  数： 	num-要查询的联动位置，读取到的联动数据存放于link指针
            *len-读取到的数据长度
            *data-读取到的数据
返 回 值：	1表示读取成功，0表示读取失败
*******************************************************************************/
u8 ReadALinkTime(u8 num , u8 *len, u8 *data)
{
	
	memcpy(data, (u8 *)&ALinkTime.Item[num].Time.delaytime[0], 5);
	*len = 5;
	return TRUE;
}

/*******************************************************************************
函 数 名：	WriteALinkTimeToTable
功能说明： 	将延时时间写入延时时间表中的num位置
参	  数： 	type是联动类型
            num表示联动位置，要写入的联动存放于data指针
返 回 值：	255表示异常，1表示成功，0表示失败
*******************************************************************************/
u8 WriteALinkTimeToTable(u8 num, u8 linknum, u8 addflag, u8 *data)
{
	return WriteALinkTime(((num==EXEERROR)?ALinkTime.Cnt:num), linknum, addflag, data);
}

/*******************************************************************************
函 数 名：	DelALinkTimeFromTable
功能说明： 	将延时时间从延时时间表中的num位置删除
参	  数： 	type是联动类型
            num表示延时时间所存的位置
返 回 值：	255表示异常，1表示成功，0表示失败
*******************************************************************************/
u8 DelALinkTimeFromTable(u8 num)
{
	return DelALinkTime(num);
}

///*******************************************************************************
//函 数 名：	CheckALinkTimeFull
//功能说明： 	检查最简联动表时间是否满
//参	  数： 	void
//返 回 值：	1表示已满，0表示不满
//*******************************************************************************/
//u8 CheckALinkTimeFull(void)
//{
//	if(ALinkTime.Cnt >= MAX_ALINKTIMEITEM)
//		return TRUE;
//	else
//		return FALSE;
//}


///*******************************************************************************
//函 数 名：	ChkALinkIsExist
//功能说明： 	匹配联动数据在联动表中是否存在，如果存在把联动号存于linknum指针
//参	  数： 	linknum-要匹配时间的联动号
//             *data-联动数据
//返 回 值：	1表示成功，0表示失败
//*******************************************************************************/
//u8 ChkALinkTimeIsExist(u8 linknum, u8 *data)
//{
//	u8 i,cnt;
//	
//	for(i=0;i<ALinkTime.Cnt;i++)
//	{
//		if(ALinkTime.Item[i].UseFlag)
//		{
//			if(ALinkTime.Item[i].linknum == linknum)
//			{
//				if(!memcmp(data, (u8 *)&ALinkTime.Item[i].Time, sizeof(DlyTimeData_t)))
//				{
//					memcpy((u8 *)&ALinkTime.Item[i].Time, data, sizeof(DlyTimeData_t));
//					return TRUE;
//				}
//			}
//		}
//	}
//	return FALSE;
//}
///*******************************************************************************
//函 数 名：	ALinkTimeClrForLinknum
//功能说明： 	从延时时间表中清除联动号相等的延时时间
//参	  数： 	linknum-要匹配时间的联动号
//返 回 值：	void
//*******************************************************************************/
//void ALinkTimeClr(void)
//{
//	if(ALinkTime.Cnt != 0)
//	{
//		memset((u8 *)&ALinkTime, 0, sizeof(ALinkTimeTable));
//		WriteDataToEEPROM(GetALinkTimeStartAddr(), sizeof(ALinkTimeTable), (u8 *)&ALinkTime);
//	}
//	
//	u8 i,cnt;
//	
//	for(i=0;i<ALinkTime.Cnt;i++)
//	{
//		if(ALinkTime.Item[i].UseFlag)
//		{
//			if(ALinkTime.Item[i].linknum == linknum)
//			{
//				if(!memcmp(data, (u8 *)&ALinkTime.Item[i].Time, sizeof(DlyTimeData_t)))
//				{
//					memcpy((u8 *)&ALinkTime.Item[i].Time, data, sizeof(DlyTimeData_t));
//					return TRUE;
//				}
//			}
//		}
//	}
//	return FALSE;
//}
///*******************************************************************************
//函 数 名：	ReadALinkTime
//功能说明： 	从最简联动时间表中读取联动时间
//参	  数： 	linknum-要查询的联动号
//            *len-读取到的数据长度,固定为5个字节，现在保留数据长度
//            *data-读取到的数据
//返 回 值：	1表示读取成功，0表示读取失败
//*******************************************************************************/
//u8 ReadALinkTime(u8 linknum , u8 *len, u8 *data)
//{
//	u8 i;
//	
//	for(i=0; i<ALinkTime.Cnt; i++)
//	{
//		if(ALinkTime.Item[i].UseFlag && ALinkTime.Item[i].linknum == linknum)
//		{
//				*data = TIMEFLAG;
//				memcpy(data+1, (u8 *)&ALinkTime.Item[i].time, sizeof(DlyTimeData_t));
//				break;
//		}
//	}
//	*len = 6;
//	return TRUE;
//}



///*******************************************************************************
//函 数 名：	DelALinkTimeItem
//功能说明： 	用联动号删除延时时间表中对应的延时时间
//参	  数： 	
//            num-联动号，唯一
//返 回 值：	1表示成功，0表示失败
//*******************************************************************************/
//u8 DelALinkTimeItem(u8 num)
//{
//	u8 number;
//	
//	if(num == 0xFF)
//	{
//		return ALinkTimeClr();
//	}
//	
//	if((number = QueryALinkTime(num)) != 0xFF)
//	{
//		return DelALinkTimeFromTable(type, number);
//	}
//	else 		//未找到联动数据
//	{
//		return TRUE;
//	}
//}
#endif
/**************************Copyright BestFu 2014-05-14*************************/
