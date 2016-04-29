 /*******************************************************************
文	件：PWM.c
说	明：PWM输出
编	译：Keil uVision4 V4.54.0.0
版	本：v1.0
编	写：Unarty
日	期：2015.01.10
********************************************************************/
#include "Sys.h"
#include "GPIO.h"
#include "Thread.h"
#include "PWM.h"

#define LED_FREP			120	

typedef struct
{
	u8 ratio;	//比例
	u8 lum;		//亮度
}LEDState_t;


static LEDState_t LEDState;
static u8 LEDRatio;
static void LED_Set(u8 ratio, u8 light);
static void LED_State2(void);
static void LED_State4(void);
static void LED_Gradual(void);

void PWM_Init(void)
{		 
	LEDRatio = 0x01;	//设置默认比例值
	RCC->APB1RSTR |= 1<<1;
	RCC->APB1RSTR &= ~(1<<1);
	RCC->APB1ENR |= 1<<1; 	//TIM3 clock enable	
	GPIOx_Cfg(GPIO_B, pin_1|pin_0, AF_PP_50M);
	/**Buzzer**/
//	AFIO->MAPR &= ~((0x07<<24)|(0x03<<10));			//清除复用IO管脚置零
//	AFIO->MAPR |= (0x02<<24)|(0x02<<10);       //设置SW调试模式，tim3-ch1复用到PB4上
//	GPIOx_Cfg(GPIO_B, pin_4, AF_PP_50M);


	TIM3->ARR = 1000000/100 - 1;	//时钟频率1KHz
	TIM3->PSC = 71;			//分频器分频到1MHz
	/*(3<<12)当TIMx_CCR1=TIMx_CNT时，翻转OC1REF的电平*/
	TIM3->CCMR2 |= (7<<12)|(1<<11)|(7<<4)|(1<<3);	// 	CH3 CH4
	TIM3->CCER |=  (0<<9)|(1<<8)|(0<<13)|(1<<12);//使能CH4/CH3PWM输出
//	/**Buzzer**/
//	TIM3->CCMR1 |= (7<<4)|(1<<3);		//CH1
//	TIM3->CCER |= (0<<1)|(1<<0);
	
	TIM3->CR1 = 0x0081;	//使能定时器2	
}

/*******************************************************************
函 数 名：	LED_State
功能说明： 	LED状态显示
参	  数： 	状态编号
返 回 值：	无
*******************************************************************/
void LED_State(u8 state)
{
	Thread_Login(FOREVER, 0, LED_FREP, LED_Gradual);
	switch (state)
	{
		case 3:
		case 1: 	LEDState.ratio = 0x13;
					LEDState.lum = 20;
					Thread_Logout(LED_State2);
					break;
		case 0: 	LEDState.ratio = 0x13;
					LEDState.lum = 20;
					Thread_Login(FOREVER, 0, LED_FREP*LEDState.lum*2 - 5, LED_State2);
					break;
		case 2:		Thread_Hang(LED_State2);
					LEDState.ratio = LEDRatio;
					LEDState.lum = 40;
					Thread_Login(FOREVER, 0, 60, LED_Gradual);
					break;
		case 4:		Thread_Hang(LED_Gradual);
					LED_Set(LEDRatio, 100);
					Thread_Login(ONCEDELAY, 0, 300, LED_State4);
					break;
		case 10:	LED_Set(LEDState.ratio, 0);
					Thread_Logout(LED_Gradual);
					
		default: 	break;
	}				
}

/*******************************************************************
函 数 名：	LED_RatioSet
功能说明： 	LED比例设置
参	  数： 	ratio:	比例值
返 回 值：	无
*******************************************************************/
void LED_RatioSet(u8 ratio)
{
	LEDRatio = ratio;
	LEDState.ratio = ratio;
}

/*******************************************************************
函 数 名：	LED_State2
功能说明： 	LED状态2
参	  数： 	状态编号
返 回 值：	无
*******************************************************************/
static void LED_State2(void)
{
	if (LEDState.ratio == 0x13)
	{
		LEDState.ratio = 0x31;
	}
	else
	{
		LEDState.ratio = 0x13;
	}
	LEDState.lum = 20;
}

///*******************************************************************
//函 数 名：	LED_State3
//功能说明： 	LED状态3
//参	  数： 	无
//返 回 值：	无
//*******************************************************************/
//static void LED_State3(void)
//{
//	u8 i = Time_Get()%50;
//	
//	LED_Set(0x01, i<<1);
//	Thread_Login(ONCEDELAY, 0, i, LED_State3);
//}

/*******************************************************************
函 数 名：	LED_State4
功能说明： 	LED状态4
参	  数： 	无
返 回 值：	无
*******************************************************************/
static void LED_State4(void)
{
	Thread_Wake(LED_Gradual);
}

/*******************************************************************
函 数 名：	LED_LumGradual
功能说明： 	LED亮度渐变
参	  数： 	无
返 回 值：	无
注    意：	渐变速度由调用频率确定
*******************************************************************/
static void LED_Gradual(void)
{
	static u8 val = 0;
	
	if (val&0x80)	//增加
	{
		if (val++ > (0x80 + LEDState.lum))
		{
    		val = LEDState.lum - 1;	
		}
	}
	else 
	{
		if (--val < 2)
		{
			val = 0x81;
		}
	}
	LED_Set(LEDState.ratio, val&0x7F);
}

/*******************************************************************
函 数 名：	LED_Set
功能说明： 	LED显示设置
参	  数： 	ratio:	颜色比例
			light:  亮度值
返 回 值：	无
*******************************************************************/
static void LED_Set(u8 ratio, u8 light)
{
	u32 i, count;
	
	if ((ratio&0xF0) && (ratio&0x0F))
	{
		count = 200;
	}
	else
	{
		count = 100;
	}
	i = (ratio>>4) + (ratio&0x0f);
	i = count*light/i;
	TIM3->CCR3 = (i*(ratio>>4));
	TIM3->CCR4 = i*(ratio&0x0F)>>1;
}
/**************************Copyright BestFu 2014-05-14*************************/
