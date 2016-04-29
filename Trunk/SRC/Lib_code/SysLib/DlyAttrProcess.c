/***************************Copyright BestFu 2016-01-28*************************
文 件：    DlyAttrProcess.c
说 明：    系统属性顺序执行处理函数
编 译：    Keil uVision V5.12.0.0
版 本：    v1.0
编 写：    jay
日 期：    2016-01-28
修  改:    无
*******************************************************************************/
#include "Command.h"
#include "BestFuLib.h"
#include "Thread.h"
#include "TimeHandle.h"
#include "WDG.h"

#if (DALAY_EXECUTE_EN > 0u)
//variable define
static DlyExeNode_t DlyExeNode = {0};       //定义延时缓冲执行队列,并初始化
static DlyExeNode_t *pDlyExeNode = &DlyExeNode;

//function declaration
static u8 DlyBufferIsEmpty(void);
static u8 DlyBufferIsFull(void);
static u8 QueryAttrRelateValue(UnitPara_t *data);
static MsgResult_t Msg_ExecuteEx(CMDPara_t *para);
static void DlyMsgProcess(void);
static u8 DeleteRelateNode(UnitPara_t *pData, DlyMsgDelType_t active);
static u8 DeleteBufferNode(u8 pos);
static void DlyRepeatHandle(DlyExeData_t *pDlyExeData, u8 *pos);

/*******************************************************************************
函 数 名：  u8 CreateBufferNode(u8 *data)
功能说明：  向延时缓冲结构中添加一个节点
参   数：   要添加的数据
返 回 值：  返回添加成功还是失败;TRUE(添加成功),FALSE(表示添加失败)
说    明：  无
*******************************************************************************/
u8 CreateBufferNode(u8 *data)
{
    static u8 xNodeHasBeenInitialised = 0u;
    u8 pos = 0u;
    if (0 == xNodeHasBeenInitialised)
    {
//      memset(&DlyExeNode,0,sizeof(DlyExeNode_t));
        xNodeHasBeenInitialised = 1u;
        Thread_Login(FOREVER, 0, TRAVERCYCLE, DlyMsgProcess); //建立延时遍历任务
    }
    if (FALSE == DlyBufferIsFull())                           //延时缓冲队列未满
    {
        pos = ((UnitPara_t *)data)->len + offsetof(UnitPara_t, data);
        DlyMsgFilterProcess((UnitPara_t *)data, RELATE_MSG_DEL);
        memcpy((u8 *)&pDlyExeNode->DlyExeData[DlyExeNode.cnt].unit, (u8 *)data, pos);
//        if (0 == *(u16 *)(&data[pos]))          			 //若延时参数为0,则清空重复参数
//        {
//            *(u16 *)(&data[pos + offsetof(DlyTimeData_t, repeat)]) = 0;
//        }
        memcpy((u8 *)&pDlyExeNode->DlyExeData[DlyExeNode.cnt].DlyTimeData.delaytime[0], data + pos, sizeof(DlyTimeData_t));
        *(u32 *)&pDlyExeNode->DlyExeData[DlyExeNode.cnt].remaindelaytime[0] = TransformTimePara((u16 *)&data[pos]) * DELAY_PRECISION / TRAVERCYCLE; //延时的精度为1s
        *(u32 *)&pDlyExeNode->DlyExeData[DlyExeNode.cnt].remainkeeptime[0] = TransformTimePara((u16 *)&data[pos + offsetof(DlyTimeData_t, keeptime)]) * KEEP_PRECISION / TRAVERCYCLE; //保持时间的精度为1s
        pDlyExeNode->DlyExeData[DlyExeNode.cnt].unitAttrRelate = QueryAttrRelateValue((UnitPara_t *)data);
        pDlyExeNode->cnt++;
		return  TRUE;
    }
    return FALSE;
}

/*******************************************************************************
函 数 名：  static u8 DlyBufferIsEmpty(void)
功能说明：     延时缓冲队列是否为空
参   数：    无
返 回 值：  TRUE(队列为空),FALSE(队列非空)
说    明：  无
*******************************************************************************/
static u8 DlyBufferIsEmpty(void)
{
    return ((0 == pDlyExeNode->cnt) ? (TRUE) : (FALSE));
}

/*******************************************************************************
函 数 名：  static u8 DlyBufferIsFull(void)
功能说明：  延时缓冲队列是否已满
参   数：   无
返 回 值：  TRUE(队列已满),FALSE(队列未满)
说    明：  无
*******************************************************************************/
static u8 DlyBufferIsFull(void)
{
    return ((pDlyExeNode->cnt >= TOTAL_Block_NUM) ? (TRUE) : (FALSE));
}

/*******************************************************************************
函 数 名：  static u8 CreateBufferNode(u8 pos)
功能说明：  向延时缓冲结构中删除一个节点
参   数：   pos:要删除节点的位置(0~TOTAL_Block_NUM)
返 回 值：  返回删除成功还是失败;TRUE(删除成功),FALSE(表示删除失败)
说    明：  无
*******************************************************************************/
static u8 DeleteBufferNode(u8 pos)
{
    if (TRUE == DlyBufferIsEmpty())     //延时缓冲队列是否为空
    {
        return  FALSE;
    }
    if (pDlyExeNode->cnt > pos)
    {
        if ((pDlyExeNode->cnt - 1) != pos) //如果要删除的位置在末尾
        {
            memcpy(&pDlyExeNode->DlyExeData[pos], &pDlyExeNode->DlyExeData[pDlyExeNode->cnt - 1], sizeof(DlyExeData_t));
        }
        memset(&pDlyExeNode->DlyExeData[pDlyExeNode->cnt - 1], 0, sizeof(DlyExeData_t));
        pDlyExeNode->cnt--;
        return  TRUE;
    }
    return  FALSE;
}

/*******************************************************************************
函 数 名：  static u8 DeleteRelateNode(UnitPara_t *data)
功能说明：  删除延时缓冲结构中关联节点
参   数：   pData：新传入的数据
            active:删除消息类型
返 回 值：  返回删除成功还是失败;TRUE(删除成功),FALSE(表示删除失败)
说    明：  无
*******************************************************************************/
static u8 DeleteRelateNode(UnitPara_t *pData, DlyMsgDelType_t active)
{
    u8 pos = 0u;
    u8 result = FALSE;
    u8 relatevalue = 0u;

    switch (active)
    {
    case SAME_MSG_DEL:
    {
        for (pos = 0; pos < pDlyExeNode->cnt; pos++)
        {
            if ((pDlyExeNode->DlyExeData[pos].unit == ((UnitPara_t *)pData)->unit) &&
                    (pDlyExeNode->DlyExeData[pos].Cmd == ((UnitPara_t *)pData)->cmd))
            {
                result = DeleteBufferNode(pos);
                pos--;
            }
        }
    }
    break;
    case RELATE_MSG_DEL:
    {
        relatevalue = QueryAttrRelateValue(pData);
        if (relatevalue > 0u)                            //关联属性值为0时,不删除任何属性
        {
            for (pos = 0; pos < pDlyExeNode->cnt; pos++)
            {
                if (pDlyExeNode->DlyExeData[pos].unit == ((UnitPara_t *)pData)->unit)
                {
					if((UNIT_PROPERTY_RELATE == relatevalue)||\
						(pDlyExeNode->DlyExeData[pos].unitAttrRelate == relatevalue))
					{
						result = DeleteBufferNode(pos);
						pos--;
					}
                }
            }
        }
    }
    break;
    default:
        break;
    }
    return (result);
}

/*******************************************************************************
函 数 名：  static u8 QueryAttrRelateValue(UnitPara_t* data)
功能说明：     查询当前单元属性关联值
参   数：    pData：新传入的数据
返 回 值：  返回当前单元属性的关联值
说    明：  无
*******************************************************************************/
static u8 QueryAttrRelateValue(UnitPara_t *para)
{
    const AttrExe_st *pAttrExe = NULL;
    u8 result = FALSE;
    u8 ExeAttrNum = 0u;
    if (para->unit >= UnitCnt_Get())
    {
        return FALSE;
    }
    pAttrExe = UnitTab[para->unit].attrExe;
    do                                              //遍历属性列表
    {
        if (pAttrExe->cmd == para->cmd)             //属性号匹配OK
        {
            result = pAttrExe->unitAttrRelate;
            break;
        }
    }
    while (((++ExeAttrNum) < UnitTab[para->unit].ExeAttrNum())   //2016.01.29 jay modify
            && ((++pAttrExe)->cmd < 0x0100));
    return (result);
}

/*******************************************************************************
函 数 名：  u8 DlyMsgFilterProcess(UnitPara_t* para)
功能说明：  过滤单元的关联消息
参   数：   pData：新传入的数据
            active:删除消息类型
返 回 值：  返回过滤的结果
说    明：  无
*******************************************************************************/
u8 DlyMsgFilterProcess(UnitPara_t *para, DlyMsgDelType_t active)
{
    if (para->unit >= UnitCnt_Get())
    {
        return FALSE;
    }
    if (TRUE == DlyBufferIsEmpty())
    {
        return FALSE;
    }
    if (0x00 != para->unit)                 //系统单元执行属性不进行消息过滤
    {
        return DeleteRelateNode(para, active);
    }
    return FALSE;
}

/*******************************************************************************
函 数 名：  u8 DeleteDlyAssMsg(u8 unit,u8 Cmd)
功能说明：     删除和当前操作相关的延时缓冲属性
参   数：    unit:单元号
            Cmd:命令号
返 回 值：  返回过滤的结果
说    明：  无
*******************************************************************************/
u8 DeleteDlyAssMsg(u8 unit, u8 Cmd)
{
    u8 data[8];
    data[0] = unit;
    data[1] = Cmd;
    data[2] = 0;
    return DlyMsgFilterProcess((UnitPara_t *)data, RELATE_MSG_DEL);
}

/*******************************************************************
函 数 名：  static MsgResult_t Msg_ExecuteEx(CMDPara_t *para)
功能说明：     消息执行
参   数：    data:   要执行的数据内容
            len;    数据长度
返 回 值：  TRUE(重复)/FALSE(不重复)
*******************************************************************/
static MsgResult_t Msg_ExecuteEx(CMDPara_t *para)
{
    MsgResult_t result = NO_CMD;
    u8 ExeAttrNum = 0u;
    const AttrExe_st *pAttrExe = UnitTab[para->unit].attrExe;

    if (para->unit < UnitCnt_Get()) //指令单元没有越界  2014.01.08 Unarty add 防止访问越界
    {
        do  //遍历属性列表
        {
            if (pAttrExe->cmd == para->cmd) //属性号匹配OK
            {
                /*写操作*/
                if (WRITEWITHACK == para->msgType
                        || WRITENACK == para->msgType
                        || WRITESUC  == para->msgType
                        || WRITEFAIL == para->msgType
                   )
                {
                    if (pAttrExe->pSet_Attr != NULL)
                    {
                        result =  pAttrExe->pSet_Attr((UnitPara_t *)&para->unit);
                    }
                }
                break;
            }//End Of if (pAttrExe->cmd == para->cmd)
        }
        while (((++ExeAttrNum) < UnitTab[para->unit].ExeAttrNum()) //2016.01.29  jay modify
                && ((++pAttrExe)->cmd < 0x0100));
    }//End of (para->unit < UnitCnt_Get())
    return result;
}

/*******************************************************************
函 数 名：  static void DlyRepeatHandle(DlyExeData_t *pDlyExeData, u8 pos)
功能说明：  延时重复执行函数
参   数：   pDlyExeData:    单个延时执行属性数据指针
            pos ：当前延时属性在延时执行链表中的位置指针
返 回 值：  无
*******************************************************************/
static void DlyRepeatHandle(DlyExeData_t *pDlyExeData, u8 *pos)
{
    if (pDlyExeData->DlyTimeData.repeat > 0u)
    {
        if (0xff != pDlyExeData->DlyTimeData.repeat)
        {
            pDlyExeData->DlyTimeData.repeat -- ;
        }
        *(u32 *)(&pDlyExeData->remaindelaytime[0]) = TransformTimePara((u16 *)(&pDlyExeData->DlyTimeData.delaytime[0])) * DELAY_PRECISION / TRAVERCYCLE;
        *(u32 *)(&pDlyExeData->remainkeeptime[0])  = TransformTimePara((u16 *)(&pDlyExeData->DlyTimeData.keeptime[0])) * KEEP_PRECISION / TRAVERCYCLE;		     
    }
    else
    {
        DeleteBufferNode(*pos);
        (*pos)--;
    }//End of if(pDlyExeData->DlyTimeData.repeat > 0u)
}

/*******************************************************************************
函 数 名：  static void DlyMsgProcess(void)
功能说明：  关联消息处理函数
参   数：   无
返 回 值：  无
说    明：  无
*******************************************************************************/
static void DlyMsgProcess(void)
{
    u8 pos = 0;
    u8 buf[20];
	
	WDG_Feed();
    if (TRUE == DlyBufferIsEmpty())	return ;
    for (pos = 0; pos < pDlyExeNode->cnt; pos ++)
    {
        if (*(u32 *)(&pDlyExeNode->DlyExeData[pos].remaindelaytime[0]) >= 1)
        {
            *(u32 *)(&pDlyExeNode->DlyExeData[pos].remaindelaytime[0]) -= 1;
            pDlyExeNode->DlyExeData[pos].delaytaskexeflag = 0;
        }
        else
        {
            *(u32 *)(&pDlyExeNode->DlyExeData[pos].remaindelaytime[0]) = 0;
            if (0 == pDlyExeNode->DlyExeData[pos].delaytaskexeflag)
            {
                //任务执行函数
                buf[0] = WRITENACK;
                memcpy(&buf[1], &pDlyExeNode->DlyExeData[pos].unit, 3 + pDlyExeNode->DlyExeData[pos].len);
                Msg_ExecuteEx((CMDPara_t *)buf);
                pDlyExeNode->DlyExeData[pos].delaytaskexeflag = 1;
                pDlyExeNode->DlyExeData[pos].keeptaskexeflag = 0;
            }
            else// if(0 != pDlyExeNode->DlyExeData[pos].delaytaskexeflag)
            {
                if (0 != CheckTimeParaIsZero(*(u16 *)(&pDlyExeNode->DlyExeData[pos].DlyTimeData.keeptime[0])))
                {
                    if (*(u32 *)(&pDlyExeNode->DlyExeData[pos].remainkeeptime[0]) >= 1)
                    {
                        *(u32 *)(&pDlyExeNode->DlyExeData[pos].remainkeeptime[0]) -= 1;
                        pDlyExeNode->DlyExeData[pos].keeptaskexeflag = 0;
                    }
                    else
                    {
                        *(u32 *)(&pDlyExeNode->DlyExeData[pos].remainkeeptime[0]) = 0;
                        if (0 == pDlyExeNode->DlyExeData[pos].keeptaskexeflag)
                        {
                            //任务执行函数
                            buf[0] = WRITENACK;
                            pDlyExeNode->DlyExeData[pos].data[0] ^= 0x01;                   //只实现开关的延时保持重复
                            memcpy(&buf[1], &pDlyExeNode->DlyExeData[pos].unit, 3 + pDlyExeNode->DlyExeData[pos].len);
                            pDlyExeNode->DlyExeData[pos].data[0] ^= 0x01;                   //只实现开关的延时保持重复
                            Msg_ExecuteEx((CMDPara_t *)buf);
                            pDlyExeNode->DlyExeData[pos].keeptaskexeflag = 1;
                            pDlyExeNode->DlyExeData[pos].delaytaskexeflag = 0;
                            DlyRepeatHandle(&pDlyExeNode->DlyExeData[pos], &pos);
                        }//End of if(0 == pDlyExeNode->DlyExeData[pos].keeptaskexeflag)
                    }//End of if(*(u16*)(&pDlyExeNode->DlyExeData[pos].remaindelaytime[0]) > TRAVERCYCLE)
                }
                else
                {
                    DlyRepeatHandle(&pDlyExeNode->DlyExeData[pos], &pos);
                }//End of if(0 != *(u16*)(&pDlyExeNode->DlyExeData[pos].DlyTimeData.keeptime[0]))
            }//End of else if(1 == pDlyExeNode->DlyExeData[pos].delaytaskexeflag)
        }//End of if(*(u16*)(&pDlyExeNode->DlyExeData[pos].remaindelaytime[0]) > TRAVERCYCLE)
    }
}
#endif
/******************************End of File************************************/
