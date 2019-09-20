#ifndef RGB_LED_H
#define RGB_LED_H

#include "nrf_drv_pwm.h"

class RgbLed {
public:
  RgbLed();
  void Init();
  void SetColor(uint8_t r, uint8_t g, uint8_t b);
private:
  nrf_drv_pwm_t pwm_ = {NRF_PWM0, NRFX_PWM0_INST_IDX};
  nrf_pwm_values_individual_t seq_values_ = {0, 0, 0, 0};
  nrf_pwm_sequence_t seq_;
};

#endif // RGB_LED_H
