/******************************Copyright BestFu 2016-03-10*****************************
�ļ���: Lark7318Buf.c
˵��  : Lark7318���ݻ��崦��
����  : Keil uVision4 V5.0
�汾  : v1.0
��д  : xjx
����  : 2016.03.10
*****************************************************************************/
#include "VoiceRecognition.h"
#include<stdlib.h>


extern StructNode_t *SYN7318Buf;

/*******************************************************************
������:	SaveData_Lark7318
����	:�Ѵ�Lark7318���յ������ݺ���Ҫ���͸�Lark7318�����ݴ浽������
����	:	u8 *data:��Ҫ�洢���ݵ���ʼ��ַ
						u8 len	:�洢����
 DataType_e dataType:�������������
����ֵ:	�Ѿ���ŵ�����֡��
*******************************************************************/ 
u8 SaveData_SYN7318(u8 *data,u8 len,DataType_e dataType)
{
	StructNode_t *h=NULL,*tmp=NULL;
	u8 cnt = 0;
	u8 i = 0;
	h = SYN7318Buf;
	while(NULL != h->Next)//�ҵ�����β��
	{
		cnt++;
		h = h->Next;
	}
	//��ʱ,h->Next == NULL
	
	tmp = (StructNode_t *)malloc(sizeof(StructNode_t));
	h->Next = tmp;
	tmp->Next = NULL;
	tmp->Next = NULL;////////test
	tmp->len = len;
	tmp->dataType = dataType;

	for(i=0; i<len; i++)
	{
		tmp->data[i] = data[i];//��������
	}

		
	return (u8)(cnt+1);
}

/*******************************************************************
������:	GetDataFromChain
����	:��˳��ӻ�����ȡ��һ֡����(FIFO:�ȴ����ȡ)
����	:	u8 *buf:ȡ�����ݵĴ�ŵ�ַ
				DataType_e dataType:��Ҫȡ������������
����ֵ:	ȡ�������ݵĳ���
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
		while((h->Next != NULL)/*&&(!flag)*/)//����datatype
		{		
			p = h;	
			h = h->Next;
			if(dataType == h->dataType)
			{
				flag = 1;//���ҳɹ�
				break;
			}			
		}
		
		if(0 == flag)
		{
			/*�������ֻʣ�������һ������û�м����*/
			h = SYN7318Buf;
			while(NULL != h->Next)//�ҵ�����β��
			{
				h = h->Next;
			}	
			if(dataType == h->dataType) 
			{
				//���������һ��
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
������:	ClrItemFromBuf
����	:���buf��Ӧ������
����	:	DataType_e dataType:��Ҫ�������������
����ֵ:	��
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


