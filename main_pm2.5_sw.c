#include "stm32f10x.h"
#include "stm32f10x_conf.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

void init_usart1(void);
void send_byte(uint8_t b);
void usart_puts(char* s);
void init_usart2(void);
void send_byte2(uint8_t b);
void usart_puts2(char* s);

void USART1_IRQHandler(void);

void BUF_Print(uint8_t* data);

uint8_t usart_buff[100] = {'\0'};
uint8_t data_buff[50] = {'\0'};
uint16_t AQI_eq[8] = {0, 50, 100, 150, 200, 300, 400, 500};
uint16_t PM25_eq[8] = {0, 12, 36, 56, 151, 251, 351, 501};
uint16_t PM10_eq[8] = {0, 55, 155, 255, 355, 425, 505, 605};

uint16_t PM1 = 0; 
uint16_t PM25 = 0;
uint16_t PM10 = 0;
float PM25_AQI = 0;
float PM10_AQI = 0;
uint8_t State = 1;
uint16_t check_sum = 0x008F;
uint8_t i = 0;
 	
volatile bool read_ok = false;

static inline void Delay_1us(uint32_t nCnt_1us)
{
  volatile uint32_t nCnt;

  for (; nCnt_1us != 0; nCnt_1us--)
    for (nCnt = 13; nCnt != 0; nCnt--);
}

static inline void Delay(uint32_t nCnt_1us)
{

			while(nCnt_1us--);
}

void USART1_IRQHandler(void)
{
    uint8_t b;
    if(USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == SET) {

          b =  USART_ReceiveData(USART1);
 	
           	switch (State)
    	{
        	case 1:
          		if (b == 0x42)
          			State++;
          		break;
        	
        	case 2:
	        	if (b == 0x4D)
          			State++;
          		else
          			State = 1;
          		break;
          	
          	case 3:
          		
          		data_buff[i++] = b;
          		if(i <=20){
          		check_sum += b;
          		}
          		if (i == 22){

          	// 		BUF_Print(data_buff);
          	// 		sprintf(usart_buff,"sum = %04x, data_sum = %04x   \n", (check_sum),((uint16_t)(data_buff[20] << 8) | data_buff[21]));
        			// usart_puts2(usart_buff);

          			if((check_sum) == ((uint16_t)(data_buff[20] << 8) | data_buff[21])){
          				PM1 = (uint16_t)(data_buff[2] << 8) | data_buff[3] ;
			          	PM25 = (uint16_t)(data_buff[4] << 8) | data_buff[5] ;
			          	PM10 = (uint16_t)(data_buff[6] << 8) | data_buff[7] ;
			          	read_ok = true;
			          	usart_puts2("sum ok \n");
          			}
          			i = 0;
          			check_sum = 0x008F;
          			State = 1;
          		}
          		break;
	        
	        default:
	        	State = 1;
	        	break;
    	}

	}
}

void send_byte(uint8_t b)
{
	/* Send one byte */
	USART_SendData(USART1, b);

	/* Loop until USART2 DR register is empty */
	while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
}


void usart_puts(char* s)
{
    while(*s) {
    	send_byte(*s);
        s++;
    }
}


void send_byte2(uint8_t b)
{
  /* Send one byte */
  USART_SendData(USART2, b);

  /* Loop until USART2 DR register is empty */
  while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
}


void usart_puts2(char* s)
{
    while(*s) {
      send_byte2(*s);
        s++;
    }
}

void init_usart1()
{

	USART_InitTypeDef USART_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Enable peripheral clocks. */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	/* Configure USART1 Rx pin as floating input. */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Configure USART1 Tx as alternate function push-pull. */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Configure the USART1 */
	USART_InitStructure.USART_BaudRate = 9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure);

	NVIC_InitTypeDef NVIC_InitStructure;

	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* Enable transmit and receive interrupts for the USART1. */
	USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

	USART_Cmd(USART1, ENABLE);

}

void init_usart2(void)
{

	USART_InitTypeDef USART_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Enable peripheral clocks. */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

	/* Configure USART1 Rx pin as floating input. */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Configure USART1 Tx as alternate function push-pull. */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Configure the USART2 */
	USART_InitStructure.USART_BaudRate = 9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx;
	USART_Init(USART2, &USART_InitStructure);

	/* Enable transmit and receive interrupts for the USART2. */
	USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
	USART_ITConfig(USART2, USART_IT_RXNE,DISABLE);

	USART_Cmd(USART2, ENABLE);
}

void BUF_Print(uint8_t* data)
{
 	int i;
    usart_puts2("\nDATA BUFFER\n");
    
    for(i=0;i<=49;i++) {
        sprintf(usart_buff,"%02X  ", data[i]);
        usart_puts2(usart_buff); 
  	}
  	usart_puts2("\n");
}

int main(void)
{
    
	init_usart1();
	init_usart2();

	usart_puts2("Init OK \n");

	while (1) {

			if(read_ok){

				sprintf(usart_buff,"PM1 = %d  PM2.5 = %d  PM10 = %d \n", PM1,PM25,PM10);
        		usart_puts2(usart_buff);
        		
        		uint8_t i,j = 0;
        		if(PM25 > 501){
        			PM25 = 501;
        		}
        		if(PM10 > 605){
        			PM10 = 605;
        		}
        		while(PM25 > PM25_eq[i]){
        			i++;
        		}
        		while(PM10 > PM10_eq[j]){
        			j++;
        		}

        		PM25_AQI = ((float)(AQI_eq[i]-AQI_eq[i-1])/(PM25_eq[i]-PM25_eq[i-1]))*PM25 + AQI_eq[i-1];
 				uint16_t int_PM25_AQI = round(PM25_AQI);
 				PM10_AQI = ((float)(AQI_eq[j]-AQI_eq[j-1])/(PM10_eq[j]-PM10_eq[j-1]))*PM10 + AQI_eq[j-1];
 				uint16_t int_PM10_AQI = round(PM10_AQI);
        		sprintf(usart_buff,"PM2.5_AQI = %d : PM10_AQI = %d\n", int_PM25_AQI, int_PM10_AQI);
        		usart_puts2(usart_buff);
        		
        		read_ok = false;

			}
	}

		
	
}
