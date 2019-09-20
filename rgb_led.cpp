#include "rgb_led.h"

#include "boards.h"

RgbLed::RgbLed() {
  seq_.values.p_individual = &seq_values_;
  seq_.length = NRF_PWM_VALUES_LENGTH(seq_values_);
  seq_.repeats = 0;
  seq_.end_delay = 0;
}

void RgbLed::Init() {
  nrf_drv_pwm_config_t config;
  config.output_pins[0] = LED_1;
  config.output_pins[1] = LED_2;
  config.output_pins[2] = LED_3;
  config.output_pins[3] = NRF_DRV_PWM_PIN_NOT_USED;
  config.irq_priority = APP_IRQ_PRIORITY_LOWEST;
  config.base_clock   = NRF_PWM_CLK_4MHz;
  config.count_mode   = NRF_PWM_MODE_UP;
  config.top_value    = 255;
  config.load_mode    = NRF_PWM_LOAD_INDIVIDUAL;
  config.step_mode    = NRF_PWM_STEP_AUTO;
  APP_ERROR_CHECK(nrf_drv_pwm_init(&pwm_, &config, NULL));
}

void RgbLed::SetColor(uint8_t r, uint8_t g, uint8_t b) {
  seq_values_.channel_0 = r;
  seq_values_.channel_1 = g;
  seq_values_.channel_2 = b;
  nrf_drv_pwm_simple_playback(&pwm_, &seq_, 3, 0);
}