/******************************Copyright BestFu 2014-05-14*****************************
	文件： UART.h
	说明： 串口操作相关函数
	编译： Keil uVision4 V4.54.0.0
	版本： v1.0
	编写： Unarty
	日期： 2014.07.03
*****************************************************************************/
#ifndef __USART_H
#define __USART_H

#include "BF_type.h"
#include "stdio.h"


#include"fifo.h"

///******************************定义调试宏**************************************/
//#define _DEBUG

//#ifdef _DEBUG
//#define  DEBUG(format,...)	printf(format,##__VA_ARGS__)
//#else
//	#define DEBUG(format,...)	
//#endif


#define USART		(2)		//串口选择

#if 	(USART == 1)
	#define UART		USART1		//?????
	#define UART_CLK	(1<<14)		//????
	
	#define UART_PORT		GPIO_A	//????
	#define UART_TX_PIN		pin_9	//??????
	#define UART_RX_PIN		pin_10	//??????
	
#elif	(USART == 2)
	#define UART		USART2		//?????
	#define UART_CLK	(1<<17)		//????
	
	#define UART_PORT		GPIO_A	//????
	#define UART_TX_PIN		pin_2	//??????
	#define UART_RX_PIN		pin_3	//??????
#elif (USART == 3)
	#define UART		USART3	
	#define UART_CLK	(1<<18)	
	#define UART_PORT		GPIO_B	
	#define UART_TX_PIN		pin_10	
	#define UART_RX_PIN		pin_11
#endif
void UART_Init(u32 pclk2, u32 bound);
u8 UART_Write(u8 c);
u8 UART_Send(u8 *data, u8 len, u8 level);
extern FIFO_t UART_RcvFifo;//串口接收数据fifo


#endif	   
/****************File End***********Copyright BestFu 2014-05-14*****************/
