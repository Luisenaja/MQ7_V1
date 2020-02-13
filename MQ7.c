#include "MQ7.h"

#define ratio_clean_air 27.5
#define RL 10.0
#define Volt_sensor 5

uint16_t adc_average =0;
uint16_t sum =0;
float Vout = 0;
float RSAir=0;
float Ro=0;
float Ro_Cal = 0;
double Ratio=0; 
float ppm=0;
uint8_t PPM;


ADC_TypeDef *adc;


static inline void Delay_1us(uint32_t nCnt_1us)
{
  volatile uint32_t nCnt;

  for (; nCnt_1us != 0; nCnt_1us--)
    for (nCnt = 13; nCnt != 0; nCnt--);
}


void MQ7_init(ADC_TypeDef *_adc){
  adc = _adc;

}


void preheat(){
    int count_heat = 10;
    int i;
    for (i=0; i<10;i++){
      Delay_1us(2000000);
      count_heat --;
      }
  }


void calibration_ro(){
 
  uint16_t adc_value = 0;
  sum =0;
  int i;
  for (i =0; i<100;i++){
      adc_value = ADC_GetConversionValue(adc);
      sum =  adc_value + sum;
      Delay_1us(1000);
      }   
    adc_average = sum/100;
    Vout = (adc_average*3.3)/4095;
    RSAir = ((Volt_sensor*RL)/Vout)-RL;
    Ro_Cal =  RSAir / ratio_clean_air;

}

void read_CO(uint8_t* _PPM){
  
  uint16_t adc_value = 0;
  sum =0;
  int i;
  Ro = Ro_Cal;
  ////// CAL RO FINSHED //////////
  for (i=0; i<10;i++){
      adc_value = ADC_GetConversionValue(adc);
      sum =  adc_value + sum;
    }   
    adc_average = sum/10;
    Vout = (adc_average*3.3)/4095; 
    RSAir = ((Volt_sensor*RL)/Vout)-RL;
    //////////GET RATIO Using Ro from calibaration
    Ratio = RSAir/Ro;
    ppm = 99.042 * (pow(Ratio, -1.518));
    PPM = ppm;
    if(ppm <= 0)
    {
      ppm=0;
    }
    if(ppm > 1000)
    {
      ppm=999;
    }
    // sprintf(buffer2, "  Ratio = %f ro = %f \n",Ratio,Ro);
    // usart_puts(buffer2);
    // sprintf(buffer2, " CO_ppm = %f  CO_PPM = %d \n",ppm,PPM);
    // usart_puts(buffer2);
    *_PPM=PPM;
    
  }
