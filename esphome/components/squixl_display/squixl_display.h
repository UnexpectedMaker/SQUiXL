#pragma once

#ifdef USE_ESP32_VARIANT_ESP32S3
#include "esphome/components/mipi_rgb/mipi_rgb.h"
#include "esphome/components/tca9555/tca9555.h"

namespace esphome {
namespace squixl_display {

class SQUiXLDisplay : public mipi_rgb::MipiRgb {
 public:
  SQUiXLDisplay(int width, int height) : MipiRgb(width, height) {}

  void setup() override;
  void dump_config() override;

  void set_expander(tca9555::TCA9555Component *expander) { this->expander_ = expander; }
  void set_init_sequence(const std::vector<uint8_t> &seq) { this->init_sequence_ = seq; }
  void set_exp_rst_pin(uint8_t pin) { this->exp_rst_pin_ = pin; }
  void set_exp_mosi_pin(uint8_t pin) { this->exp_mosi_pin_ = pin; }
  void set_exp_clk_pin(uint8_t pin) { this->exp_clk_pin_ = pin; }
  void set_exp_cs_pin(uint8_t pin) { this->exp_cs_pin_ = pin; }

 protected:
  void hardware_reset_();
  void write_9bit_(bool dc, uint8_t data);
  void write_init_sequence_();

  tca9555::TCA9555Component *expander_{nullptr};
  std::vector<uint8_t> init_sequence_;
  uint8_t exp_rst_pin_{1};
  uint8_t exp_mosi_pin_{2};
  uint8_t exp_clk_pin_{3};
  uint8_t exp_cs_pin_{4};
};

}  // namespace squixl_display
}  // namespace esphome
#endif
