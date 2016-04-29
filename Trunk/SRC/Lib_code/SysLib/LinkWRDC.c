/***************************Copyright BestFu 2014-05-14*************************
文	件：	LinkWRDC.c
说	明：	分项的写入、读取、删除和查询工作源文件
编	译：	Keil uVision4 V4.54.0.0
版	本：	v1.0
编	写：	Joey
日	期：	2013.7.22 
修　改：	暂无
*******************************************************************************/
#include "linkall.h"
/*******************************************************************************
函 数 名：	CheckLinkFull
功能说明： 	检查联动表是否已满?
参	  数： 	type是联动类型
返 回 值：	255表示异常，1表示成功，0表示失败
*******************************************************************************/
#if (ALINK_EN > 0 || SLINK_EN > 0 || CLINK_EN > 0)
u8 CheckLinkFull(u8 type)
{
	switch(type)
	{
#if(ALINK_EN > 0)
		case SIMPLESTLINK: return CheckALinkFull(); 
#endif
#if(SLINK_EN > 0)
		case SIMPLELINK:   return CheckSLinkFull();
#endif
#if(CLINK_EN > 0)
		case COMPLEXLINK:  return CheckCLinkFull();
#endif
		default:		   return EXEERROR;
	}
}

/*******************************************************************************
函 数 名：	QuerySLink
功能说明： 	查询data联动号在简单联动表中的位置
参	  数： 	data-要查询的联动号,type是联动类型
返 回 值：	255表示异常，1表示成功，0表示失败
*******************************************************************************/
u8 QueryLink(u8 type, u8 num)
{
	switch(type)
	{
#if(ALINK_EN > 0)
		case SIMPLESTLINK: return QueryALink(num); 
#endif
#if(SLINK_EN > 0)
		case SIMPLELINK:   return QuerySLink(num); 
#endif
#if(CLINK_EN > 0)
		case COMPLEXLINK:  return QueryCLink(num); 
#endif
		default:           return EXEERROR;
	}
}

/*******************************************************************************
函 数 名：	QueryAllLinkFromTable
功能说明： 	查询联动表中的所有联动号
参	  数： 	type是联动类型，查询到的联动号存放于data指针
返 回 值：	255表示异常，1表示成功，0表示失败
*******************************************************************************/
u8 QueryAllLinkFromTable(u8 type, u8 *len, u8 *data)
{
	u8 i;
	switch(type)
	{
#if(ALINK_EN > 0)
		case SIMPLESTLINK:
			for(i=0;i<ALink.Cnt;i++)
			{
				if(ALink.linkitem[i].UseFlag)
				{
					data[i] = ALink.linkitem[i].LinkNum;
				}
			}
			*len = ALink.Cnt;
			return TRUE;
#endif
#if(SLINK_EN > 0)
		case SIMPLELINK:
			for(i=0;i<SLink.Cnt;i++)
			{
				if(SLink.linkitem[i].UseFlag)
				{
					data[i] = SLink.linkitem[i].LinkNum;
				}
			}
			*len = SLink.Cnt;
			return TRUE;
#endif
#if(CLINK_EN > 0)
		case COMPLEXLINK:
			for(i=0;i<CLink.Cnt;i++)
			{
				if(CLink.linkitem[i].UseFlag)
				{
					data[i] = CLink.linkitem[i].LinkNum;
				}
			}
			*len = CLink.Cnt;
			return TRUE;
#endif
		default:           
			return EXEERROR;
	}
}

/*******************************************************************************
函 数 名：	ReadLinkFromTable
功能说明： 	从联动表中用联动号读取联动
参	  数： 	type是联动类型
            num表示联动位置，读取到的联动存放于data指针
返 回 值：	255表示异常，1表示成功，0表示失败
*******************************************************************************/
u8 ReadLinkFromTable(u8 type, u8 num, u8 *len, u8 *data)
{
	switch(type)
	{
#if(ALINK_EN > 0)
		case SIMPLESTLINK: return ReadALink(num,len,data); 
#endif
#if(SLINK_EN > 0)
		case SIMPLELINK:   return ReadSLink(num,len,data); 
#endif
#if(CLINK_EN > 0)
		case COMPLEXLINK:  return ReadCLink(num,len,data); 
#endif
		default:           return EXEERROR;
	}
}

/*******************************************************************************
函 数 名：	WriteLinkToTable
功能说明： 	将联动写入联动表中的num位置
参	  数： 	type是联动类型
            num表示联动位置，要写入的联动存放于data指针
返 回 值：	255表示异常，1表示成功，0表示失败
*******************************************************************************/
u8 WriteLinkToTable(u8 type, u8 num, u8 linknum, u8 addflag, u8 *data)
{
	switch(type)
	{
#if(ALINK_EN > 0)
		case SIMPLESTLINK: 
			if(WriteALink(((num==EXEERROR)?ALink.Cnt:num), linknum, addflag, data))
			{
				return TRUE;
			}
			else
			{
				return FALSE;
			}
#endif
#if(SLINK_EN > 0)
		case SIMPLELINK:
			if(WriteSLink(((num==EXEERROR)?SLink.Cnt:num), linknum, addflag, data))
			{
				return TRUE;
			}
			else
			{
				return FALSE;
			}
#endif
#if(CLINK_EN > 0)
		case COMPLEXLINK:
			if(WriteCLink(((num==EXEERROR)?CLink.Cnt:num), linknum, addflag, data))
			{
				return TRUE;
			}
			else
			{
				return FALSE;
			}
#endif
		default:           
			return EXEERROR;
	}
}

/*******************************************************************************
函 数 名：	DelAllLinkFromTable
功能说明： 	删除type类型的所有联动
参	  数： 	type是联动类型
返 回 值：	255表示异常，1表示成功，0表示失败
*******************************************************************************/
u8 DelAllLinkFromTable(u8 type)
{
	switch(type)
	{
#if(ALINK_EN > 0)
		case SIMPLESTLINK:
			memset((u8 *)&ALink, 0, sizeof(ALinkTable));
	#if (ALINK_SRC_VALUE_EN > 0)
			memset((u8 *)&ALinkV,0,sizeof(ALinkVTable));
	#endif
			WriteDataToEEPROM(GetALinkStartAddr(), 
				(u32)sizeof(ALinkTable), (u8 *)&ALink);
			return TRUE;
#endif
#if(SLINK_EN > 0)
		case SIMPLELINK:
			memset((u8 *)&SLink, 0, sizeof(SLinkTable));
			WriteDataToEEPROM(GetSLinkStartAddr(), 
				(u32)sizeof(SLinkTable), (u8 *)&SLink);
			DelAllSLinkValue();//刷新联动是否执行过表
			SLinkSrcInit();//删除联动源执行表
			return TRUE;
#endif
#if(CLINK_EN > 0)
		case COMPLEXLINK:
			memset((u8 *)&CLink, 0, sizeof(CLinkTable));
			WriteDataToEEPROM(GetCLinkStartAddr(), 
				(u32)sizeof(CLinkTable), (u8 *)&CLink);
			DelAllCLinkSrc();//刷新复杂条件表
			return TRUE;
#endif
		default:           
			return EXEERROR;
	}
}

/*******************************************************************************
函 数 名：	DelAllLinkFromTable
功能说明： 	删除type类型的num位置联动
参	  数： 	type是联动类型
返 回 值：	255表示异常，1表示成功，0表示失败
*******************************************************************************/
u8 DelLinkFromTable(u8 type, u8 num)
{
	switch(type)
	{
#if(ALINK_EN > 0)
		case SIMPLESTLINK: 
			if(num < MAX_ALINKITEM)
			{
				return DelALink(num);
			}
			else
				return FALSE;
#endif
#if(SLINK_EN > 0)
		case SIMPLELINK:
			if(num < MAX_SLINKITEM)
			{
				return DelSLink(num);
			}
			else
				return FALSE;
#endif
#if(CLINK_EN > 0)
		case COMPLEXLINK:
			if(num < MAX_CLINKITEM)
			{
				return DelCLink(num);
			}
			else
				return FALSE;
#endif
		default:           
			return EXEERROR;
	}
}

/*******************************************************************************
函 数 名:  u8 ChkLinkIsExist(u8 type, u8 *linknum, u8 *data)
功能说明:  从type类型中查找是否存在相同的联动，存在的话把linknum赋值
参    数:  *linknum-找到的最小联动号存放于此
           type-类型
           *data-联动数据
返 回 值:  1表示成功，0表示失败
*******************************************************************************/
u8 ChkLinkIsExist(u8 type, u8 *linknum, u8 *data)
{
	switch(type)
	{
#if(ALINK_EN > 0)
		case SIMPLESTLINK:
			return ChkALinkIsExist(linknum, data);
#endif
#if(SLINK_EN > 0)
		case SIMPLELINK:
			return ChkSLinkIsExist(linknum, data);
#endif
#if(CLINK_EN > 0)
		case COMPLEXLINK:
			return ChkCLinkIsExist(linknum, data);
#endif
		default:           
			return EXEERROR;
	}
}

/*******************************************************************************
函 数 名：	GetLinkNum
功能说明： 	从type类型中获取对应新联动号值
参	  数： 	*linknum-找到的最小联动号存放于此
             type-类型
返 回 值：	1表示成功，0表示失败
*******************************************************************************/
u8 GetLinkNum(u8 type, u8 *linknum)
{
	switch(type)
	{
#if(ALINK_EN > 0)
		case SIMPLESTLINK:
			return GetALinkNum(linknum);
#endif
#if(SLINK_EN > 0)
		case SIMPLELINK:
			return GetSLinkNum(linknum);
#endif
#if(CLINK_EN > 0)
		case COMPLEXLINK:
			return GetCLinkNum(linknum);
#endif
		default:           
			return EXEERROR;
	}
}

/*******************************************************************************
函 数 名：	WriteLinkItem
功能说明： 	将联动数据写入type类型联动表中，写完更新回应参数区数据，用于应答
参	  数： 	*data-联动数据，数据同SLinkItem结构。
返 回 值：	1表示成功，0表示失败
*******************************************************************************/
u8 WriteLinkItem(u8 type, u8 *data)
{
	u8 num, linknum, addflag=0;
	if(ChkLinkIsExist(type, &linknum, data) == 0)//是否存在，存在则给到linknum值
	{//不存在
		if(CheckLinkFull(type) == TRUE)
		{
			return LINKFULL;
		}
		
		if(GetLinkNum(type, &linknum) == TRUE)
		{
			addflag = 1;//新增各项联动计数器
			num = EXEERROR;
		}
		else
		{
			return LINKNUM_ERR;
		}
		if (WriteLinkToTable(type, num, linknum, addflag, data) != 1)
		{
			return EEPROM_ERR;
		}
	}
	else
	{
#if(ALINKTIME_EN > 0)		//遍历时间T，如果存在相同时间T删除，返回
		u8 number;
		if((number = QueryALinkTime(num)) != EXEERROR)
		{
			return DelALinkTimeFromTable(number);
		}
#endif
	}

	data -= 2;
	data[0] = linknum;
	
	return COMPLETE;
}

/*******************************************************************************
函 数 名：	ModifyLinkItem
功能说明： 	将联动号所处的联动数据写入联动表，如果联动表存在该联动号所对应的联动，
            直接覆盖，如果联动表中不存在，则在后面新添该条联动
参	  数： 	type-联动类型，linknum-联动号，*data-联动数据，数据同SLinkItem结构。
返 回 值：	1表示成功，0表示失败
*******************************************************************************/
u8 ModifyLinkItem(u8 type, u8 linknum, u8 *data)
{
	u8 num,flag=0,addflag=0;
#if (ALINKTIME_EN > 0)
	u8 number;
#endif
	if((num = QueryLink(type, linknum)) == EXEERROR)
	{
		addflag = 1;
	}

	flag = WriteLinkToTable(type, num, linknum, addflag, data);

#if (ALINKTIME_EN > 0)
	if((number = QueryALinkTime(linknum)) != 0xFF)
	{
		return DelALinkTimeFromTable(number);
	}
#endif	
	return flag;
}

/*******************************************************************************
函 数 名：	ReadSLinkItem
功能说明： 	从type类型联动表中读出对应的联动数据，存放于data指针
参	  数： 	num-联动号，读到的联动数据放于data指针
            type为类型号分最简联动、简单联动和复杂联动三种
            *len-读取到的数据长度
返 回 值：	1表示成功，0表示失败
*******************************************************************************/
u8 ReadLinkItem(u8 type, u8 num, u8 *len, u8 *data)
{
	u8 number;
#if (ALINKTIME_EN > 0)	
	u8 cnt,time[6]={0};
	ALinkItem *pLink;
#endif
	if((number = QueryLink(type,num)) != EXEERROR)
	{
		ReadLinkFromTable(type, number, len, data);
		
#if (ALINKTIME_EN > 0)	
		pLink = (ALinkItem *)(data-2);
		
		if( (number = QueryALinkTime(num)) != EXEERROR)
		{
			ReadALinkTime(number , &cnt, &time[1]);
			
			*len += sizeof(DlyTimeData_t) + 1;
			time[0] = 1;
			memcpy(&pLink->data[pLink->DestLen], &time[0], sizeof(DlyTimeData_t)+1);
		}
#endif
		
		return TRUE;
	}
	return FALSE;
}

/*******************************************************************************
函 数 名：	QueryLinkItem
功能说明： 	用联动号查询简单联动表中是否配有联动
参	  数： 	*data-读到的联动数据放于data指针
            type-类型，分别为最简、简单、复杂三种类型
            num-联动号，唯一
返 回 值：	1表示成功，0表示失败
*******************************************************************************/
u8 QueryLinkItem(u8 type, u8 num, u8 *len, u8 *data)
{
	if(num == 0xFF)
	{
		QueryAllLinkFromTable(type, len, data);
		return TRUE;
	}
	else if((data[0] = QueryLink(type, num)) != EXEERROR)
	{
		*len = 1;
		return TRUE;
	}
	return FALSE;
}

/*******************************************************************************
函 数 名：	DelLinkItem
功能说明： 	用联动号删除type类型联动表中对应联动
参	  数： 	type-类型，分别为最简、简单、复杂三种类型
            num-联动号，唯一
返 回 值：	1表示成功，0表示失败
*******************************************************************************/
u8 DelLinkItem(u8 type, u8 num)
{
	u8 number;
	
	if(num == 0xFF)
	{
#if (ALINKTIME_EN > 0)
		AllALinkTimeClr();
#endif
		return DelAllLinkFromTable(type);
	}
	
	if((number = QueryLink(type, num)) != 0xFF)
	{
		DelLinkFromTable(type, number);
	}
	
#if (ALINKTIME_EN > 0)
	if((number = QueryALinkTime(num)) != 0xFF)
	{
		return DelALinkTimeFromTable(number);
	}
#endif	
	return TRUE;
}

#if (ALINKTIME_EN > 0)
/*******************************************************************************
函 数 名：	WriteLinkItemEx
功能说明： 	将联动数据写入type类型联动表中，写完更新回应参数区数据，用于应答
参	  数： 	*data-联动数据，数据同SLinkItem结构。
返 回 值：	1表示成功，0表示失败
*******************************************************************************/
u8 WriteLinkItemEx(u8 type, u8 *data)
{
	u8 num, linknum, addflag=0;
	u8 time[5]={0};

	ALinkItem *pLink;
	pLink = (ALinkItem *)(data-2);
	
	if(pLink->data[pLink->DestLen] == TIMEFLAG)//flag有效时，复制时间，否则为清0
	{
		memcpy(time, (u8 *)&pLink->data[pLink->DestLen+1], sizeof(DlyTimeData_t));
		if(FALSE == CheckAllTimeParaIsOK((DlyTimeData_t*)time,1))
		{
			return TIME_PARA_ERR;
		}
	}
	if(ChkLinkIsExist(type, &linknum, data) == 0)//是否存在，存在则给到linknum值
	{//不存在
		if(CheckLinkFull(type) == TRUE)
		{
			return LINKFULL;
		}
		
		if(GetLinkNum(type, &linknum) == TRUE)
		{
			addflag = 1;//新增各项联动计数器
			num = EXEERROR;
		}
		else
		{
			return LINKNUM_ERR;
		}
		if (WriteLinkToTable(type, num, linknum, addflag, data) != 1)
		{
			return EEPROM_ERR;
		}
	}
	else
	{//存在
		if( (num = QueryALinkTime(linknum)) == EXEERROR)
		{
			addflag = 1;//新增
		}
	}
	
	WriteALinkTimeToTable(num, linknum, addflag, time);
	
	data -= 2;
	data[0] = linknum;
	
	return COMPLETE;
}
/*******************************************************************************
函 数 名：	ModifyLinkItemEx
功能说明： 	将联动号所处的联动数据写入联动表，如果联动表存在该联动号所对应的联动，
            直接覆盖，如果联动表中不存在，则在后面新添该条联动
参	  数： 	type-联动类型，linknum-联动号，*data-联动数据，数据同SLinkItem结构。
返 回 值：	1表示成功，0表示失败
*******************************************************************************/
u8 ModifyLinkItemEx(u8 type, u8 linknum, u8 *data)
{
	u8 num,flag=0,addflag=0;
	u8 time[5]={0};

	ALinkItem *pLink;
	pLink = (ALinkItem *)(data-2);

	if(pLink->data[pLink->DestLen] == TIMEFLAG)//flag有效时，复制时间，否则为清0
	{
		memcpy(time, (u8 *)&pLink->data[pLink->DestLen+1], sizeof(DlyTimeData_t));
		if(FALSE == CheckAllTimeParaIsOK((DlyTimeData_t*)time,1))
		{
			return TIME_PARA_ERR;
		}
	}
	
	if((num = QueryLink(type, linknum)) == EXEERROR)
	{
		addflag = 1;
	}

	flag = WriteLinkToTable(type, num, linknum, addflag, data);
	
	if( (num = QueryALinkTime(linknum)) == EXEERROR)
	{
		addflag = 1;//新增
	}
	WriteALinkTimeToTable(num, linknum, addflag, time);
	
	return flag;
}
///*******************************************************************************
//函 数 名：	ReadSLinkItemEx
//功能说明： 	从type类型联动表中读出对应的联动数据，存放于data指针
//参	  数： 	num-联动号，读到的联动数据放于data指针
//            type为类型号分最简联动、简单联动和复杂联动三种
//            *len-读取到的数据长度
//返 回 值：	1表示成功，0表示失败
//*******************************************************************************/
//u8 ReadLinkItemEx(u8 type, u8 num, u8 *len, u8 *data)
//{
//	u8 number;
//	u8 cnt,time[6]={0};
//	ALinkItem *pLink;
//	
//	if((number = QueryLink(type,num)) != EXEERROR)
//	{
//		ReadLinkFromTable(type, number, len, data);

//		pLink = (ALinkItem *)(data-2);
//		ReadALinkTime(num , &cnt, &time[1]);
//		if(!(time[0] == 0 &&
//			 time[1] == 0 &&
//			 time[2] == 0 &&
//			 time[3] == 0 &&
//			 time[4] == 0	))
//		{
//			*len += sizeof(DlyTimeDat_t) + 1;
//			time[0] = 1;
//			memcpy(&pLink->data[pLink->DestLen], &time[1], sizeof(DlyTimeDat_t));
//		}
//		
//		return TRUE;
//	}
//	return FALSE;
//}
///*******************************************************************************
//函 数 名：	DelLinkItemEx
//功能说明： 	用联动号删除type类型联动表中对应联动
//参	  数： 	type-类型，分别为最简、简单、复杂三种类型
//            num-联动号，唯一
//返 回 值：	1表示成功，0表示失败
//*******************************************************************************/
//u8 DelLinkItemEx(u8 type, u8 num)
//{
//	u8 number;
//	
//	if(num == 0xFF)
//	{
//		AllALinkTimeClr();
//		return DelAllLinkFromTable(type);
//	}
//	
//	if((number = QueryLink(type, num)) != 0xFF)
//	{
//		DelLinkFromTable(type, number);
//	}
//	
//	if((number = QueryALinkTime(num)) != 0xFF)
//	{
//		return DelALinkTimeFromTable(number);
//	}
//	
//	return TRUE;
//}
#endif

/*******************************************************************************
函 数 名：	DelLinkForUnit
功能说明： 	按单元号删除联动
参	  数： 	unitnum-单元号
返 回 值：	1表示成功，0表示失败
*******************************************************************************/
u8 DelLinkForUnit(u8 unitnum)
{
	u8 i=0,updateflag=0;

	if(!ALink.Cnt)
	{
		return 1;
	}

	while(ALink.Cnt > i)
	{
		if(unitnum == ALink.linkitem[i].SUnitID)
		{
			updateflag = 1;
			memcpy((u8 *)&ALink.linkitem[i], (u8 *)&ALink.linkitem[ALink.Cnt-1], sizeof(ALinkItem));
			ALink.Cnt--;
			
#if (ALINKTIME_EN > 0)
			DelLinkItem(0, ALink.linkitem[i].LinkNum);
#endif
		}
		else
		{
			i++;
		}
	}
	
	if(updateflag)
	{
		WriteDataToEEPROM(GetALinkStartAddr(), sizeof(ALinkTable), 
			(u8 *)&ALink);
	}
	return 1;
}
/*******************************************************************************
函 数 名：	DelLinkForPropIDandVal
功能说明： 	按属性号和源值删除联动
参	  数： 	propID-属性号，sValue-源值
返 回 值：	1表示成功，0表示失败
*******************************************************************************/
u8 DelLinkForPropIDandVal(u8 sPropID , int sValue) // yanhuan adding 2015/11/02
{
	u8 i=0 ;
	for( i = 0 ; i < ALink.Cnt ;)
	{
		if(ALink.linkitem[i].SPropID == sPropID && 
			*(u32*)&ALink.linkitem[i].SValue == sValue)
		{
			DelALink(i);
#if (ALINKTIME_EN > 0)
			DelLinkItem(0, ALink.linkitem[i].LinkNum);
#endif
			continue ;
		}
		i++ ;
	}		
	return 1;
}

#endif
/**************************Copyright BestFu 2014-05-14*************************/
