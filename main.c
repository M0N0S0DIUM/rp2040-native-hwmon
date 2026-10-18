#include <math.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "tusb.h"
#define R_FIXED 10000.0f
#define R_NOMINAL 10000.0f
#define BETA_WATER 3950.0f
#define BETA_AIR 3950.0f
#define T_NOMINAL_K 298.15f
#define SENSOR_OFFSET 0x2f
static float sample_celsius(unsigned channel){
 adc_select_input(channel);(void)adc_read();uint32_t total=0;
 for(int i=0;i<64;i++){total+=adc_read();sleep_us(30);}
 float fraction=(float)total/(64.0f*4095.0f);
 if(fraction<0.005f || fraction>0.995f)return NAN;
 float r_ntc=R_FIXED*fraction/(1.0f-fraction);
 float kelvin=1.0f/(1.0f/T_NOMINAL_K+logf(r_ntc/R_NOMINAL)/(channel==0?BETA_WATER:BETA_AIR));
 float c=kelvin-273.15f;
 return c< -20.0f || c>110.0f?NAN:c;
}
static void write_be16(uint8_t*p,int16_t v){p[0]=(uint16_t)v>>8;p[1]=(uint16_t)v&255;}
int main(void){
 adc_init();adc_gpio_init(26);adc_gpio_init(27);tusb_init();
 absolute_time_t next=get_absolute_time();
 while(true){
  tud_task();
  if(absolute_time_diff_us(get_absolute_time(),next)<=0){
   next=make_timeout_time_ms(1000);
   float water=sample_celsius(0),air=sample_celsius(1);
   uint8_t report[64]={0};report[0]=1;
   write_be16(&report[SENSOR_OFFSET],isfinite(water)?(int16_t)lroundf(water*100):0x7fff);
   write_be16(&report[SENSOR_OFFSET+2],isfinite(air)?(int16_t)lroundf(air*100):0x7fff);
   for(int i=2;i<4;i++)write_be16(&report[SENSOR_OFFSET+2*i],0x7fff);
   if(tud_hid_ready())tud_hid_report(1,report+1,63);
  }
 }
}
