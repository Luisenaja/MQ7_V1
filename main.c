#include "stm32f10x.h"
#include "stm32f10x_conf.h"
#include <stdio.h>
#include <stdlib.h>
void send_byte(uint8_t b);
void usart_puts(char* s);
int adc_average =0;
int sum =0;
float Vout = 0;
float Co_out = 0;

static inline void Delay_1us(uint32_t nCnt_1us)
{
  volatile uint32_t nCnt;

  for (; nCnt_1us != 0; nCnt_1us--)
    for (nCnt = 13; nCnt != 0; nCnt--);
}

void gpio_toggle(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);
void gpio_toggle(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
	GPIOx->ODR ^= GPIO_Pin;
}

static inline void Delay(uint32_t nCnt_1us)
{

			while(nCnt_1us--);
}
/* Refer to stm32f10x_adc.c */
#define ADC1_DR_ADDRESS                  ((uint32_t)0x4001244C)

/* Store sampled value here  */
volatile uint32_t ADCConvertedValue;

void init_usart1(void);
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
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; // set interrupt group
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  /* Enable transmit and receive interrupts for the USART1. */
  USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
  // USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

  USART_Cmd(USART1, ENABLE);

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


void init_adc(void);
void init_adc()
{
  DMA_InitTypeDef DMA_InitStructure;
  ADC_InitTypeDef ADC_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;
   /* ADCCLK = PCLK2/4 */
  RCC_ADCCLKConfig(RCC_PCLK2_Div6);
  /* Enable peripheral clocks ------------------------------------------------*/
  /* Enable DMA1 clock */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

  /* Enable ADC1 and GPIOC clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);


  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

  //Configure LED Pin
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = 	GPIO_Mode_AIN;  //AIN =  analog input
  GPIO_InitStructure.GPIO_Pin = 	GPIO_Pin_0; 	
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* DMA1 channel1 configuration ----------------------------------------------*/
  DMA_DeInit(DMA1_Channel1);
  DMA_InitStructure.DMA_PeripheralBaseAddr = ((uint32_t)0x4001244C);
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&ADCConvertedValue;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
  DMA_InitStructure.DMA_BufferSize = 1;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
  DMA_Init(DMA1_Channel1, &DMA_InitStructure);
  
  /* Enable DMA1 channel1 */
  DMA_Cmd(DMA1_Channel1, ENABLE);

  // ADC_TempSensorVrefintCmd(ENABLE);   

  /* ADC1 configuration ------------------------------------------------------*/
  ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
  ADC_InitStructure.ADC_ScanConvMode = ENABLE;
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfChannel = 1;
  ADC_Init(ADC1, &ADC_InitStructure);

  /* ADC1 regular channel16 configuration */ 
  ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_71Cycles5); 

  /* Enable ADC1 DMA */
  ADC_DMACmd(ADC1, ENABLE);
  
  /* Enable ADC1 */
  ADC_Cmd(ADC1, ENABLE);

  /* Enable ADC1 reset calibration register */   
  ADC_ResetCalibration(ADC1);
  /* Check the end of ADC1 reset calibration register */
  while(ADC_GetResetCalibrationStatus(ADC1));

  /* Start ADC1 calibration */
  ADC_StartCalibration(ADC1);
  /* Check the end of ADC1 calibration */
  while(ADC_GetCalibrationStatus(ADC1));
     
  /* Start ADC1 Software Conversion */ 
  ADC_SoftwareStartConvCmd(ADC1, ENABLE);

}

void setup_LEDb(void);
void setup_LEDb(){
	GPIO_InitTypeDef GPIO_InitStructure;

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

	//Configure LED Pin
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = 	GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = 	GPIO_Pin_13; 	
	GPIO_Init(GPIOC, &GPIO_InitStructure);
}






int16_t adc_value = 0;
int main(void)
{
  init_adc();
  init_usart1();
  //setup_LEDb();
  char buffer[80] = {'\0'};
  char buffer2[80] = {'\0'};


while (1) {
    
    int i=0;
    float RSAir=0;
    float Ro=0;
    float RS=0;
    float Ratio=0;
    float ppm=0;

    for (i =0; i<5;i++){
    	adc_value = ADC_GetConversionValue(ADC1);
    	sum =  adc_value + sum; 
      Delay_1us(2000000);
    }
    //i = 0;

    adc_average = sum/5;
    Vout = (adc_average*3.3)/4095;


    RSAir = (((3.3 *10 ) / Vout)-10);
    if(RSAir < 0)
    {
      RSAir =0;
    }

    Ro = RSAir/27.5;
    if (Ro < 0)
    {
      Ro = 0;
    }

    RS = (((3.3 *10 ) / Vout)-10);
    if(RS < 0)
    {
      RS = 0;
    }

    Ratio = RS / Ro;
    if(Ratio <= 0 || Ratio >100)
    {
      Ratio = 0.01;
    }

    ppm = 99.014 * (pow(Ratio, -1.518));

    if(ppm <= 0)
    {
      ppm=0;
    }
    if(ppm > 1000)
    {
      ppm=999;
    }

    sprintf(buffer2, "Vout : %f RSAir = %f Ro = %f \n",Vout,RSAir,Ro );
    sprintf(buffer, "PPMout : %f \n",ppm);
    usart_puts(buffer2); 
    usart_puts(buffer); 

	}

    // if (co_out <= 500 && co_out > 10 ) {
    // 	sprintf(buffer, "ADC_Value: %d \n",adc_average);
    // 	sprintf(buffer2, "co_out: %.02f \n",Vout);
    // 	usart_puts(buffer);
    // 	usart_puts(buffer2);
    // 	Delay_1us(100000);
    // 	sum = 0;
    // 	Vout = 0;
    // 	adc_average = 0;
    // 	adc_value = 0;
    // }
    // else {
    // 	 usart_puts("Out_of_range");
    // 	 sum = 0;
    // 	 Vout = 0;
    // 	 adc_average = 0;
    // 	 adc_value = 0;
    // }

}
