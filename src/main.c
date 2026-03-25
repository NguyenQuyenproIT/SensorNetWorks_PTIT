#include "stm32f10x.h"                  // Device header
#include "stm32f10x_gpio.h"             // Keil::Device:StdPeriph Drivers:GPIO
#include "stm32f10x_i2c.h"              // Keil::Device:StdPeriph Drivers:I2C
#include "stm32f10x_rcc.h"              // Keil::Device:StdPeriph Drivers:RCC
#include "stm32f10x_usart.h"            // Keil::Device:StdPeriph Drivers:USART
#include "stdio.h"
#include "string.h"

/* UART1: APB2
TX: PA9 - AF_PP
RX: PA10 - IN_FLOATING
*/

#define BH1750_ADDR 0x46 // dia chi I2C cua sensor
#define BH1750_PWR_ON 0x01 // bat nguon 
#define BH1750_RESET  0x07 // reset (khoi dong lai)
#define BH1750_CONT_HRES_MODE 0x10 // mode do lien tuc

void delay_ms(uint16_t time){ // 1s
		uint16_t i;
			for(i = 0; i<time; i++){
					SysTick -> CTRL = 0x00000005;
					SysTick -> LOAD = 72000-1;
					SysTick -> VAL = 0;
			while(!(SysTick -> CTRL & (1 << 16))){}			
			}
}

// UART
void config_uart(){
		GPIO_InitTypeDef gpio_pin;
		USART_InitTypeDef usart_init;
	
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);
	
	// pin USART
		gpio_pin.GPIO_Pin = GPIO_Pin_9; // TX
		gpio_pin.GPIO_Mode = GPIO_Mode_AF_PP;
		gpio_pin.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOA, &gpio_pin);
	
		gpio_pin.GPIO_Pin = GPIO_Pin_10; // RX
		gpio_pin.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		gpio_pin.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOA, &gpio_pin);

			usart_init.USART_BaudRate = 115200;
			usart_init.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
			usart_init.USART_WordLength = USART_WordLength_8b;
			usart_init.USART_Parity = USART_Parity_No;
			usart_init.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
			
			USART_Init(USART1, &usart_init); // configure to register: Baudrate, Word length, Parity, Stop bit, Flow control, Mode… but still not turn on USART.
			USART_Cmd(USART1, ENABLE); // bat bo USART. -> complete
}	
	
struct __FILE{
int handle; 
};
FILE __stdout;

int fputc(int ch, FILE *f) {
    USART_SendData(USART1, (uint8_t) ch);
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
    return ch;
}

void Config_I2C(){
		GPIO_InitTypeDef GPIO_init;
		I2C_InitTypeDef I2C_init;
	
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); // cau hinh pin I2C
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE); // Bat bo I2C-1
	
		GPIO_init.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
		GPIO_init.GPIO_Mode = GPIO_Mode_AF_OD;
		GPIO_init.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOB, &GPIO_init);
	
		I2C_init.I2C_ClockSpeed = 100000; // Toc do xung clock SCL = 100kHz.
		I2C_init.I2C_Mode = I2C_Mode_I2C;
		I2C_init.I2C_DutyCycle = I2C_DutyCycle_2; // Nghia là ti le xung clock = T_high / T_low = 2 (mac dinh cho 100kHz).
		I2C_init.I2C_OwnAddress1 = 0x00; // stm32 is master, enough not care
		I2C_init.I2C_Ack = I2C_Ack_Enable; // Nghia là sau khi nhan 1 byte, STM32 se tra ACK cho slave -> báo hieu "tôi nhan roi, gui tiep di".
		I2C_init.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit; // almost sensor (BH1750) deu dùng 7-bit address, nên chon 7bit.
		I2C_Init(I2C1, &I2C_init);
		
		I2C_Cmd(I2C1, ENABLE);
}


/* START
 Send Address
 Wait ACK
 Send Data
 STOP
*/
void I2C_WriteByte(uint8_t address, uint8_t data){ // gui 1 byte du lieu toi thiet bi i2c co dia chi "address"
	// Send START
  I2C_GenerateSTART(I2C1, ENABLE); // tao tin hieu start

  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT)); // doi den khi xac nhan START duoc tao thanh cong
	// khi start duoc tao thanh cong, bit SB (start bit) duoc set (stm tro thanh master và gui dia chi cho slave)
	
  I2C_Send7bitAddress(I2C1, address, I2C_Direction_Transmitter); // gui dia chi + chon read/write
	/*
			r/w = 1 thi master read(receive) data, slave send
			r/w = 0 thi master gui(ghi) data, slave read (receive)
	*/

  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)); // kiem tra slave ACK (ghi)
	
	// I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED - doc
	
  I2C_SendData(I2C1,data); // Gui du lieu cho I2C1 - gui byte du lieu
  while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED)); // Cho gui xong du lieu
	
	I2C_GenerateSTOP(I2C1, ENABLE); // tao stop
}

void BH1750_Init(void) {
    delay_ms(10);
    I2C_WriteByte(BH1750_ADDR, BH1750_PWR_ON); //(0x01, kich hoat)
    delay_ms(10);
    I2C_WriteByte(BH1750_ADDR, BH1750_CONT_HRES_MODE); // 0x10, do lien tuc, do phan giai cao
    delay_ms(200); 
}

//float BH1750_ReadLight(){
uint16_t BH1750_ReadLight(){
  uint16_t value = 0;
	uint8_t lsb;
	uint8_t msb;
	
    I2C_GenerateSTART(I2C1, ENABLE);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
	
    I2C_Send7bitAddress(I2C1, BH1750_ADDR, I2C_Direction_Receiver);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED)); 

		//Doc du lieu 2 byte
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED));
    msb = I2C_ReceiveData(I2C1); 

    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED));
    lsb = I2C_ReceiveData(I2C1); 

    I2C_AcknowledgeConfig(I2C1, DISABLE); 
    I2C_GenerateSTOP(I2C1, ENABLE); 
    I2C_AcknowledgeConfig(I2C1, ENABLE); 

    value = ((msb << 8) | lsb) / 1.2; 
    return value;
}

uint16_t receive;
//float receive;
char buffer[40];

int main(){
	
	Config_I2C();
	config_uart();
	BH1750_Init();
	delay_ms(200);

	
	while(1){
		receive = BH1750_ReadLight();		
//		printf("LUX value: %f \n", receive);
		printf("LUX value: %d \n", receive);
		delay_ms(600);		
}
}