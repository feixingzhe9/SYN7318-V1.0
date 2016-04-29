/******************************Copyright BestFu 2016-03-10*****************************
文件名: Lark7318Buf.c
说明  : Lark7318数据缓冲处理
编译  : Keil uVision4 V5.0
版本  : v1.0
编写  : xjx
日期  : 2016.03.10
*****************************************************************************/
#include "VoiceRecognition.h"
#include<stdlib.h>


extern StructNode_t *SYN7318Buf;

/*******************************************************************
函数名:	SaveData_Lark7318
描述	:把从Lark7318接收到的数据和需要发送给Lark7318的数据存到缓冲区
参数	:	u8 *data:需要存储数据的起始地址
						u8 len	:存储长度
 DataType_e dataType:存入的数据类型
返回值:	已经存放的数据帧数
*******************************************************************/ 
u8 SaveData_SYN7318(u8 *data,u8 len,DataType_e dataType)
{
	StructNode_t *h=NULL,*tmp=NULL;
	u8 cnt = 0;
	u8 i = 0;
	h = SYN7318Buf;
	while(NULL != h->Next)//找到链表尾部
	{
		cnt++;
		h = h->Next;
	}
	//此时,h->Next == NULL
	
	tmp = (StructNode_t *)malloc(sizeof(StructNode_t));
	h->Next = tmp;
	tmp->Next = NULL;
	tmp->Next = NULL;////////test
	tmp->len = len;
	tmp->dataType = dataType;

	for(i=0; i<len; i++)
	{
		tmp->data[i] = data[i];//保存数据
	}

		
	return (u8)(cnt+1);
}

/*******************************************************************
函数名:	GetDataFromChain
描述	:按顺序从缓存中取出一帧数据(FIFO:先存的先取)
参数	:	u8 *buf:取出数据的存放地址
				DataType_e dataType:需要取出的数据类型
返回值:	取出的数据的长度
*******************************************************************/ 
u8 GetDataFromChain(u8 *buf,DataType_e dataType)
{
	StructNode_t *h=NULL,*p = NULL;//(StructNode_t*)malloc(sizeof(StructNode_t));
	u8 num = 0;
	u8 i = 0;
	u8 flag = 0;//?????
	
	if(SYN7318BufLen > 0)
	{	
		h = SYN7318Buf;
		p = SYN7318Buf;
		while((h->Next != NULL)/*&&(!flag)*/)//查找datatype
		{		
			p = h;	
			h = h->Next;
			if(dataType == h->dataType)
			{
				flag = 1;//查找成功
				break;
			}			
		}
		
		if(0 == flag)
		{
			/*到这里就只剩链表最后一个数据没有检查了*/
			h = SYN7318Buf;
			while(NULL != h->Next)//找到链表尾部
			{
				h = h->Next;
			}	
			if(dataType == h->dataType) 
			{
				//数据在最后一个
				num = h->len;
				if(0 != num)
				{
					for(i=0; (i<num)&&(num<20); i++)
					{
						buf[i] =  h->data[i];//* = ;= i
					}
					p->Next = h->Next;
					free(h);
					return num;
				}				
			}
//			return 0;//????
		}			
			
		if(1 == flag)
		{
			num = h->len;
			if(0 != num)
			{
				for(i=0; (i<num)&&(num<20); i++)
				{
					buf[i] =  h->data[i];//* = ;= i
				}
				p->Next = h->Next;
				free(h);
				return num;
			}
//			else
//			{
//				return 0;
//			}		
		}
//		Lark7318BufLen--;	
	}
	return 0;	
}

/*******************************************************************
函数名:	ClrItemFromBuf
描述	:清除buf对应的数据
参数	:	DataType_e dataType:需要清除的数据类型
返回值:	无
*******************************************************************/ 
void ClrItemFromBuf(DataType_e dataType)
{
	u8 len;
	u8 data[BUF_DATA_LEN];
	while((len = GetDataFromChain(data,dataType)) != 0)
	{
		
	}
}

/****************File End***********Copyright BestFu 2016-03-16*****************/


