// STM32F411CEU6 WeAct Black Pill: 4 x NTC -> Aquacomputer Farbwerk HID status.
#include "bsp/board_api.h"
#include "stm32f4xx.h"
#include "tusb.h"
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#define SENSOR_OFFSET 0x2f
#define NTC_R25_OHMS 10000.0f
#define PULLUP_OHMS 10000.0f
// Nominal 10k/B3950 thermistors. Tune individual beta values if needed.
#define WATER_BETA 3950.0f
#define AIR_BETA 3950.0f
#define CHANNEL3_BETA 3950.0f
#define CHANNEL4_BETA 3950.0f
#define INVALID_TEMPERATURE 0x7fffu
#define ADC_SAMPLE_COUNT 32

// TinyUSB's Black Pill BSP uses PA2/PA3 for USART2 TX/RX. Reconfigure all
// PA1-PA4 as ADC inputs AFTER board_init(). Each needs a 10k pull-up to 3V3.
// Without a pull-up on an unwired channel, its ADC input floats and may show
// a spurious temperature rather than N/A. Install pull-ups for all 4 channels
// now if you want to plug optional sensors in later without reflashing.
static void adc_start(void) {
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
  RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
  for (unsigned channel = 1u; channel <= 4u; ++channel) {
    const uint32_t mode_mask = 3u << (2u * channel);
    GPIOA->MODER = (GPIOA->MODER & ~mode_mask) | mode_mask;
    GPIOA->PUPDR &= ~mode_mask;
  }
  // APB2=84 MHz, ADC clock prescaler /4 gives 21 MHz (within ADC limits).
  ADC->CCR = (ADC->CCR & ~ADC_CCR_ADCPRE) | ADC_CCR_ADCPRE_0;
  ADC1->CR1 = 0u;
  ADC1->CR2 = 0u;
  ADC1->SQR1 = 0u;  // sequence length 1
  // 480 ADC cycles on ADC_IN1...ADC_IN4 to settle the NTC dividers.
  for (unsigned channel = 1u; channel <= 4u; ++channel) {
    const uint32_t shift = 3u * channel;
    ADC1->SMPR2 = (ADC1->SMPR2 & ~(7u << shift)) | (7u << shift);
  }
  ADC1->CR2 = ADC_CR2_ADON;
  for (volatile unsigned delay = 0; delay < 10000u; ++delay) { __NOP(); }
}

static bool adc_one(unsigned channel, uint16_t* output) {
  ADC1->SQR3 = channel;             // ADC_IN1...IN4 = PA1...PA4
  ADC1->SR = 0u;                   // reset EOC, OVR
  ADC1->CR2 |= ADC_CR2_SWSTART;
  uint32_t timeout = 100000u;
  while (!(ADC1->SR & ADC_SR_EOC) && --timeout) {}
  if (!timeout) return false;
  *output = (uint16_t)(ADC1->DR & 0x0fffu);
  return true;
}

// Returns centidegrees C or 0x7fff for invalid/open/shorted sensors.
// Kernel scales centidegrees * 10 into hwmon's millidegrees Celsius.
static uint16_t ntc_centidegrees(unsigned adc_channel, float beta) {
  uint16_t raw;
  // Discard first conversion after channel switch.
  if (!adc_one(adc_channel, &raw)) return INVALID_TEMPERATURE;
  uint32_t sum = 0u;
  for (unsigned i=0; i < ADC_SAMPLE_COUNT; ++i) {
    if (!adc_one(adc_channel, &raw)) return INVALID_TEMPERATURE;
    sum += raw;
  }
  const float frac = (float)sum / (float)(4095u * ADC_SAMPLE_COUNT);
  if (frac <= 0.005f || frac >= 0.995f) return INVALID_TEMPERATURE;
  const float r_thermistor = PULLUP_OHMS * frac / (1.0f - frac);
  const float kelvin = 1.0f /
    (1.0f / 298.15f + logf(r_thermistor / NTC_R25_OHMS) / beta);
  const float celsius = kelvin - 273.15f;
  // Aquacomputer driver reads unsigned 16-bit temperatures.
  if (!isfinite(celsius) || celsius < 0.0f || celsius > 100.0f)
    return INVALID_TEMPERATURE;
  return (uint16_t)lroundf(celsius * 100.0f);
}

static inline void store_be16(uint8_t* dst, uint16_t n) {
  dst[0] = (uint8_t)(n >> 8);
  dst[1] = (uint8_t)n;
}

static void temperature_report(void) {
  uint8_t report[64] = {0};
  report[0] = 0x01;
  // Report byte positions include the HID report ID at byte 0.
  store_be16(&report[SENSOR_OFFSET], ntc_centidegrees(1u, WATER_BETA));
  store_be16(&report[SENSOR_OFFSET + 2], ntc_centidegrees(2u, AIR_BETA));
  store_be16(&report[SENSOR_OFFSET + 4], ntc_centidegrees(3u, CHANNEL3_BETA));
  store_be16(&report[SENSOR_OFFSET + 6], ntc_centidegrees(4u, CHANNEL4_BETA));
  if (tud_hid_ready()) {
    (void)tud_hid_report(1u, report + 1, sizeof(report) - 1);
  }
}

int main(void) {
  board_init();
  adc_start();
  tusb_rhport_init_t init = { .role = TUSB_ROLE_DEVICE, .speed = TUSB_SPEED_AUTO };
  tusb_init(BOARD_TUD_RHPORT, &init);
  if (board_init_after_tusb) board_init_after_tusb();
  uint32_t previous = board_millis();
  bool led = false;
  while (1) {
    tud_task();
    const uint32_t now = board_millis();
    if ((uint32_t)(now - previous) >= 1000u) {
      previous = now;
      temperature_report();
      board_led_write(led);
      led = !led;
    }
  }
}

uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id,
                              hid_report_type_t report_type, uint8_t* buffer,
                              uint16_t reqlen) {
  (void)instance; (void)report_id; (void)report_type; (void)buffer;
  (void)reqlen;
  return 0;
}

void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id,
                            hid_report_type_t report_type,
                            uint8_t const* buffer, uint16_t len) {
  (void)instance; (void)report_id; (void)report_type; (void)buffer; (void)len;
}
