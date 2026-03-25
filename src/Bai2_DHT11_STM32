#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_tim.h"
#include <stdio.h>


void PIN_MODE_UART1(){
		
	GPIO_InitTypeDef gpio_pin;
	USART_InitTypeDef usart_init;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	// TX
	gpio_pin.GPIO_Pin = GPIO_Pin_9;
	gpio_pin.GPIO_Mode = GPIO_Mode_AF_PP;
	gpio_pin.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &gpio_pin);

	// RX
	gpio_pin.GPIO_Pin = GPIO_Pin_10;
	gpio_pin.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	gpio_pin.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &gpio_pin);

	usart_init.USART_BaudRate = 115200;
	usart_init.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	usart_init.USART_WordLength = USART_WordLength_8b;
	usart_init.USART_StopBits = USART_StopBits_1;
	usart_init.USART_Parity = USART_Parity_No;
	usart_init.USART_HardwareFlowControl = USART_HardwareFlowControl_None;

	USART_Init(USART1, &usart_init);
	USART_Cmd(USART1, ENABLE);
}


struct __FILE{
	int handle;
};

FILE __stdout;

int fputc(int ch, FILE *f){
	
	USART_SendData(USART1, (uint8_t) ch);
	while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
	
	return ch;
}

void Timer2_Init(void){

	TIM_TimeBaseInitTypeDef timerInit;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	timerInit.TIM_CounterMode = TIM_CounterMode_Up;
	timerInit.TIM_Period = 0xFFFF;
	timerInit.TIM_Prescaler = 72 - 1;

	TIM_TimeBaseInit(TIM2, &timerInit);
	TIM_Cmd(TIM2, ENABLE);
}

void delay_us(uint16_t us){
	
	TIM_SetCounter(TIM2,0);
	while(TIM_GetCounter(TIM2) < us);
}

void delay_ms(uint16_t ms){
	
	while(ms--)
		delay_us(1000);
}


void GPIO_Config(void){

	GPIO_InitTypeDef gpio;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	gpio.GPIO_Pin = GPIO_Pin_12;
	gpio.GPIO_Mode = GPIO_Mode_IPU;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;

	GPIO_Init(GPIOB, &gpio);
}


void DHT11_Start(void){

	GPIO_InitTypeDef gpio;

	gpio.GPIO_Pin = GPIO_Pin_12;
	gpio.GPIO_Mode = GPIO_Mode_Out_OD;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&gpio);

	GPIO_ResetBits(GPIOB,GPIO_Pin_12);
	delay_ms(20);

	GPIO_SetBits(GPIOB,GPIO_Pin_12);
	delay_us(35);

	gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB,&gpio);
}

uint8_t DHT11_Read_Byte(void){

	uint8_t i,byte=0;

	for(i=0;i<8;i++)
	{
		while(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_12)==0);

		TIM_SetCounter(TIM2,0);

		while(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_12)==1);

		if(TIM_GetCounter(TIM2) > 45)
			byte = (byte<<1) | 1;
		else
			byte = (byte<<1);
	}

	return byte;
}

uint8_t DHT11_Read_Data(uint8_t *hum_int,uint8_t *hum_dec,uint8_t *temp_int,uint8_t *temp_dec){

	uint8_t byte[5];
	uint8_t i;
	uint32_t timeout=0;

	while(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_12)==1){
		delay_us(1);
		if(++timeout>100) return 0;
	}

	timeout=0;
	while(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_12)==0){
		delay_us(1);
		if(++timeout>100) return 0;
	}

	timeout=0;
	while(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_12)==1){
		delay_us(1);
		if(++timeout>100) return 0;
	}

	for(i=0;i<5;i++)
		byte[i]=DHT11_Read_Byte();

	*hum_int = byte[0];
	*hum_dec = byte[1];
	*temp_int = byte[2];
	*temp_dec = byte[3];

	return byte[4];
}


int main(){

	uint8_t hum_int,hum_dec,temp_int,temp_dec,checksum;

	PIN_MODE_UART1();
	Timer2_Init();
	GPIO_Config();
	delay_ms(1000);
	while(1){

		DHT11_Start();

		checksum = DHT11_Read_Data(&hum_int,&hum_dec,&temp_int,&temp_dec);

		if((hum_int+hum_dec+temp_int+temp_dec)==checksum){

		printf("Humidity: %d.%d %%  ", hum_int, hum_dec);
		printf("Temperature: %d.%d \xB0""C\r\n", temp_int, temp_dec);
			
		}
		else{
			printf("Loi checksum!\r\n");
		}

		delay_ms(2000);
	}
}
