#include "stm32f10x.h"                  // Device header
#include "stm32f10x_gpio.h"             // Keil::Device:StdPeriph Drivers:GPIO
#include "stm32f10x_spi.h"              // Keil::Device:StdPeriph Drivers:SPI
#include "stm32f10x_usart.h"            // Keil::Device:StdPeriph Drivers:USART
#include <stdio.h>
#include <string.h>

// PIN CONFIGURE
/* RFID RC522
PA4 -> (CS)
PA5 -> SCK
PA6 -> MISO
PA7 -> MOSI
PA3 -> RST
*/


#define RC522_CS_LOW()     GPIOA->BRR  = GPIO_BRR_BR4
#define RC522_CS_HIGH()    GPIOA->BSRR = GPIO_BSRR_BS4

#define RC522_RST_LOW()    GPIOA->BRR  = GPIO_BRR_BR3
#define RC522_RST_HIGH()   GPIOA->BSRR = GPIO_BSRR_BS3


// RC522 REGISTER MAP

#define CommandReg      0x01
#define CommIEnReg      0x02
#define CommIrqReg      0x04
#define ErrorReg        0x06
#define FIFODataReg     0x09
#define FIFOLevelReg    0x0A
#define ControlReg      0x0C
#define BitFramingReg   0x0D
#define ModeReg         0x11
#define TxControlReg    0x14
#define TxASKReg        0x15
#define TModeReg        0x2A
#define TPrescalerReg   0x2B
#define TReloadRegL     0x2D
#define TReloadRegH     0x2C
#define VersionReg      0x37


//   RC522 COMMAND

#define PCD_IDLE        0x00
#define PCD_TRANSCEIVE  0x0C
#define PCD_RESETPHASE  0x0F

#define PICC_REQIDL     0x26
#define PICC_ANTICOLL   0x93

#define PICC_HALT 0x50

#define RFCfgReg 0x26


// delay 1ms system tick
void delay_ms(unsigned int time){	
		unsigned int i;
			for(i = 0; i<time; i++){
					SysTick -> CTRL = 0x00000005;
					SysTick -> LOAD = 72000-1;
					SysTick -> VAL = 0;
			while(!(SysTick -> CTRL & (1 << 16))){}			
			}
}

void PIN_MODE_UART1(){
		
		GPIO_InitTypeDef gpio_pin;
		USART_InitTypeDef usart_init;
		NVIC_InitTypeDef NVIC_InitStructure;

	
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	
	// pin USART
		gpio_pin.GPIO_Pin = GPIO_Pin_9;
		gpio_pin.GPIO_Mode = GPIO_Mode_AF_PP;
		gpio_pin.GPIO_Speed = GPIO_Speed_50MHz; // TX
		GPIO_Init(GPIOA, &gpio_pin);
	
		gpio_pin.GPIO_Pin = GPIO_Pin_10;
		gpio_pin.GPIO_Mode = GPIO_Mode_IN_FLOATING; // RX
		gpio_pin.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOA, &gpio_pin);
	
		usart_init.USART_BaudRate = 115200;
		usart_init.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
		usart_init.USART_WordLength = USART_WordLength_8b;
		usart_init.USART_StopBits = USART_StopBits_1;
		usart_init.USART_Parity = USART_Parity_No;
		usart_init.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
			
		
		USART_Init(USART1, &usart_init); // configure to register: Baudrate, Word length, Parity, Stop bit, Flow control, Mode… but still not turn on USART.
			
		USART_ITConfig(USART1, USART_IT_RXNE, ENABLE); // enable ngat RXNE
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

// cau hình spi

void SPI1_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    SPI_InitTypeDef  SPI_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

    /* SCK */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* MOSI */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* MISO */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* CS */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* RST */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    RC522_CS_HIGH();
    RC522_RST_HIGH();

    /* SPI CONFIG */
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;

    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;

    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_128;

    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7;

    SPI_Init(SPI1, &SPI_InitStructure);
    SPI_Cmd(SPI1, ENABLE);
}

uint8_t SPI1_Transfer(uint8_t data)
{
    while(!(SPI1->SR & SPI_SR_TXE));
    SPI1->DR = data;
    while(!(SPI1->SR & SPI_SR_RXNE));
    return SPI1->DR;
}


//  RC522 LOW LEVEL

void RC522_WriteReg(uint8_t addr, uint8_t val)
{
    RC522_CS_LOW();
    SPI1_Transfer((addr<<1)&0x7E);
    SPI1_Transfer(val);
    RC522_CS_HIGH();
}

uint8_t RC522_ReadReg(uint8_t addr)
{
    uint8_t val;
    RC522_CS_LOW();
    SPI1_Transfer(((addr<<1)&0x7E)|0x80);
    val = SPI1_Transfer(0x00);
    RC522_CS_HIGH();
    return val;
}

void RC522_Reset(void)
{
    RC522_RST_LOW();
    delay_ms(5);
    RC522_RST_HIGH();
    delay_ms(50);

    RC522_WriteReg(CommandReg, PCD_RESETPHASE);
    delay_ms(50);
}

void RC522_AntennaOn(void)
{
    uint8_t temp = RC522_ReadReg(TxControlReg);
    if(!(temp & 0x03))
        RC522_WriteReg(TxControlReg, temp | 0x03);
}


//   RC522 INIT

void RC522_Init(void)
{
    RC522_Reset();

    RC522_WriteReg(TModeReg, 0x80);
    RC522_WriteReg(TPrescalerReg, 0xA9);
    RC522_WriteReg(TReloadRegL, 0xE8);
    RC522_WriteReg(TReloadRegH, 0x03);

    RC522_WriteReg(TxASKReg, 0x40);
    RC522_WriteReg(ModeReg, 0x3D);

    RC522_WriteReg(RFCfgReg, 0x7F);  // Max gain

    RC522_AntennaOn();
}

// TRANSCEIVE CORE FUNCTION
uint8_t bitFraming;
uint8_t RC522_Transceive(uint8_t *sendData,
                         uint8_t sendLen,
                         uint8_t *backData,
                         uint8_t *backLen)
{
    uint8_t fifoLevel;
    uint8_t lastBits;
    uint8_t n;
    uint16_t i;

    RC522_WriteReg(CommIEnReg, 0x77 | 0x80);
    RC522_WriteReg(CommandReg, PCD_IDLE);
    RC522_WriteReg(CommIrqReg, 0x7F);
    RC522_WriteReg(FIFOLevelReg, 0x80);

    for(i = 0; i < sendLen; i++)
        RC522_WriteReg(FIFODataReg, sendData[i]);

    RC522_WriteReg(CommandReg, PCD_TRANSCEIVE);

    /* CH? b?t StartSend, KHÔNG phá BitFraming */
    bitFraming = RC522_ReadReg(BitFramingReg);
    RC522_WriteReg(BitFramingReg, bitFraming | 0x80);

    i = 2000;
    do{
        n = RC522_ReadReg(CommIrqReg);
        i--;
    }while(i && !(n & 0x30));

    RC522_WriteReg(BitFramingReg, bitFraming);   // restore

    if(i == 0)
        return 2;

    if(RC522_ReadReg(ErrorReg) & 0x1B)
        return 3;

    fifoLevel = RC522_ReadReg(FIFOLevelReg);
    lastBits  = RC522_ReadReg(ControlReg) & 0x07;

    if(lastBits)
        *backLen = (fifoLevel - 1) * 8 + lastBits;
    else
        *backLen = fifoLevel * 8;

    for(i = 0; i < fifoLevel; i++)
        backData[i] = RC522_ReadReg(FIFODataReg);

    return 0;
}

//  REQUEST + ANTICOLL

uint8_t RC522_Request(uint8_t *tagType)
{
    uint8_t backLen;
    uint8_t reqMode = PICC_REQIDL;

    RC522_WriteReg(BitFramingReg, 0x07);
    RC522_WriteReg(FIFOLevelReg, 0x80);   // Clear FIFO

    if(RC522_Transceive(&reqMode, 1, tagType, &backLen) != 0)
        return 1;

    if(backLen != 16)   // ATQA ph?i = 16 bit
        return 1;

    return 0;
}

uint8_t RC522_AntiColl(uint8_t *uid)
{
    uint8_t backLen;
    uint8_t serNum[2] = {PICC_ANTICOLL, 0x20};
    uint8_t i, check = 0;

    RC522_WriteReg(BitFramingReg, 0x00);

    if(RC522_Transceive(serNum, 2, uid, &backLen) != 0)
        return 1;

    if(backLen != 40)   // 5 bytes = 40 bits
        return 1;

    for(i = 0; i < 4; i++)
        check ^= uid[i];

    if(check != uid[4])
        return 1;

    return 0;
}

void RC522_Halt(void)
{
    uint8_t buffer[2];
    uint8_t backLen;

    buffer[0] = PICC_HALT;
    buffer[1] = 0x00;

    RC522_WriteReg(BitFramingReg, 0x00);
    RC522_Transceive(buffer, 2, buffer, &backLen);

    RC522_WriteReg(CommandReg, PCD_IDLE);
    RC522_WriteReg(FIFOLevelReg, 0x80);    // Clear FIFO
}

uint8_t tagType[2];
uint8_t uid[5];
uint8_t i;


int main(void)
{
    PIN_MODE_UART1();
    SPI1_Init();
    RC522_Init();
	
	printf("START!\n");

while(1)
{
    if(!RC522_Request(tagType) && !RC522_AntiColl(uid))
    {
        printf("UID: ");
        for(i=0;i<4;i++)
            printf("%02X ",uid[i]); // fix
        printf("\r\n");

        RC522_Halt();

        while(!RC522_Request(tagType))
            delay_ms(50);
    }

    delay_ms(50);
}

}
