/******************************Copyright BestFu 2014-05-14*****************************
	文件： UART.c
	说明： 串口操作相关函数
	编译： Keil uVision4 V4.54.0.0
	版本： v1.0
	编写： Unarty
	日期： 2014.07.03
*****************************************************************************/
#include "uart.h"	
#include "GPIO.h"
#include "Lark7318.h"

#include "VoiceRecognition.h"

#include "Thread.h"




#include "Lark7318DataAnalyze.h"
#include "LED.h"



RcvFrom7318_t RcvFrom7318tmp;

UartDataProcess_t UartDataProcess;





u8 SaveData_SYN7318(u8 *data,u8 len,DataType_e dataType);
u8 SYN7318BufLen = 0;//串口接收链表里的数据帧数
void DataTestFun(void);//测试函数
void TestFunForSYN7318(void);//测试函数


//FIFO_t UART_RcvFifo;//串口接收数据fifo
//u8 UART_RcvBuf[1024];
/*******************************************************************
函 数 名：	UART_Init
功能说明：	串口初始化
参	  数：	pclk2： 时钟频率（单位M）
			bound： 通信波特率
返 回 值：	无
*******************************************************************/ 
void UART_Init(u32 pclk2, u32 bound)
{
//	u8 a[7] = {0};
//	u8 len = 7;
//	u8 i = 0;
#if 	(USART == 1)	
	RCC->APB2RSTR 	|= UART_CLK;   	
	RCC->APB2RSTR 	&= ~(UART_CLK);	
	RCC->APB2ENR  	|= UART_CLK;  	
#elif	(USART == 2)	
	RCC->APB1RSTR	|= UART_CLK;   	
	RCC->APB1RSTR	&= ~(UART_CLK);	
	RCC->APB1ENR	|= UART_CLK;  
#elif   (USART == 3)	
	RCC->APB1RSTR	|= UART_CLK;   	
	RCC->APB1RSTR	&= ~(UART_CLK);	
	RCC->APB1ENR	|= UART_CLK;  
#endif
	GPIOx_Cfg(UART_PORT, UART_TX_PIN, AF_PP_50M);  
	GPIOx_Cfg(UART_PORT, UART_RX_PIN, IN_UPDOWN);  

#if 	(USART == 1)	
 	UART->BRR = (pclk2*1000000)/(bound); 
#elif	(USART == 2)
	UART->BRR = (pclk2*1000000/2)/(bound); 
#elif   (USART == 3)
	UART->BRR = (pclk2*1000000/2)/(bound);
#endif

	UART->CR1|=(1<<13)|(1<<3)|(1<<2)|(1<<8); 		
	UART->CR1|=(1<<5);  					
	
#if 	(USART == 1)		  				   
	MY_NVIC_Init( 2, 1, USART1_IRQn, 2);
#elif	(USART == 2) 		     
	MY_NVIC_Init( 2, 1, USART2_IRQn, 2);
#elif  (USART == 3) 
	MY_NVIC_Init( 2, 2, USART3_IRQn, 2);
#endif


}




/*******************************************************************
函 数 名：	UART_Send
功能说明：	串口数据发送
参	  数：	*data: 要发送的数内容
			len :	数据长度
			level:  串口选择
返 回 值：	发送结果 TRUE/FALSE
*******************************************************************/ 
u8 UART_Send(u8 *data, u8 len, u8 level)
{
//	u8 a[20] = {0};
//	u8 i = 0;
//	for(i=0;i<len;i++)
//	{
//		a[i] = data[i];
//	}
		
	while (len--)
	{
		if (!UART_Write(*data++))
		{
//			UART_Init(72, 115200);
		}
	}
	return TRUE;
}

/*******************************************************************
函 数 名：	UART_WriteBit
功能说明：	串口写入一个字节数据 
参	  数：	ch:	要写入的数据
返 回 值：	写入结果TRUE/FALSE
*******************************************************************/ 
u8 UART_Write(u8 c)
{   
	u32 t;
	
	for (t = 0; ((UART->SR&0X40) == 0)&&(t < 20000); t++)   //等待数据发送完毕
	{}	
		
	if (t < 20000)  //未超时
	{
		UART->DR = (u8)c;
		return TRUE;
	}   
	return FALSE;
}
#if 	(USART == 1)
/*******************************************************************
函 数 名：	USART1_IRQHandler
功能说明：	串口中断处理
参	  数：	无
返 回 值：	无
*******************************************************************/ 
void USART1_IRQHandler(void)
{
	if(UART->SR&(1<<5))//接收到数据
	{	

	}
}
#elif	(USART == 2) 
/*******************************************************************
函 数 名：	USART2_IRQHandler
功能说明：	串口中断处理
参	  数：	无
返 回 值：	无
*******************************************************************/ 
void USART2_IRQHandler(void)
{

	
//////////////////////////////////////	
	if(UART->SR&(1<<5))//接收到数据
	{
		RcvFrom7318tmp.data[RcvFrom7318tmp.num] = UART->DR;
		RcvFrom7318tmp.num++;		
		UartDataProcess.timecnt = 0;//串口数据接收 时间复位
	}
///////////////////////////////////////////////	
}

#elif	(USART == 3)
/*******************************************************************
函 数 名：	USART3_IRQHandler
功能说明：	串口中断处理
参	  数：	无
返 回 值：	无
*******************************************************************/ 
void USART3_IRQHandler(void)
{

} 
#endif

/*******************************************************************
函 数 名：	UartRcvProcess
功能说明：	判断一帧串口数据是否接收完成
参	  数：	无
返 回 值：	无
说明		:		10ms调用一次
*******************************************************************/ 
void UartRcvProcess(void)
{
  static u8 cnt;
	cnt++;
	cnt %= 255;
	UartDataProcess.timecnt++;
	if(UartDataProcess.timecnt > UART_DATA_RDY_TIME)
	{
		if(RcvFrom7318tmp.num > 0)//
		{
			if(SYN7318BufLen < 20)//限制链表总的数据帧数，防止堆溢出造成复位
			{
				SYN7318BufLen = SaveData_SYN7318(RcvFrom7318tmp.data,RcvFrom7318tmp.num,DataRcv);
			}			
		}
		
		RcvFrom7318tmp.num = 0;//一帧数据接收完成，下一次就要重新开始	
	}
}


//__asm void wait()
//{
//      BX lr
//}
/** 
  * @brief  This function handles Hard Fault exception. 
  * @param  None 
  * @retval None 
  */  
void HardFault_Handler(void)  
{ 
//		 wait();
  /* Go to infinite loop when Hard Fault exception occurs */  
  if (CoreDebug->DHCSR & 1) {  //check C_DEBUGEN == 1 -> Debugger Connected  
      __breakpoint(0);  // halt program execution here         
  }  
  while (1)  
  {  
  }  
} 
/****************File End***********Copyright BestFu 2014-05-14*****************/





