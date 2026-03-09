#ifdef USE_ESP32_VARIANT_ESP32S3
#include "squixl_display.h"
#include "esphome/core/log.h"
#include <driver/gpio.h>
#include <esp_task_wdt.h>

namespace esphome {
namespace squixl_display {

static const char *const TAG = "squixl_display";
static const uint8_t DELAY_FLAG = 0xFF;

void SQUiXLDisplay::setup() {
  ESP_LOGCONFIG(TAG, "Setting up SQUiXL Display...");
  esp_task_wdt_reset();

  // Configure expander SPI pins as output
  this->expander_->pin_mode(this->exp_mosi_pin_, gpio::FLAG_OUTPUT);
  this->expander_->pin_mode(this->exp_clk_pin_, gpio::FLAG_OUTPUT);
  this->expander_->pin_mode(this->exp_cs_pin_, gpio::FLAG_OUTPUT);

  // Hardware reset the LCD via expander
  this->hardware_reset_();

  // Set initial SPI line states
  this->expander_->digital_write(this->exp_cs_pin_, true);
  this->expander_->digital_write(this->exp_clk_pin_, false);
  this->expander_->digital_write(this->exp_mosi_pin_, false);

  // Bit-bang the ST7701S init sequence through the expander
  this->write_init_sequence_();

  // Reset GT911 touch controller after display SPI init
  // DevOS does this before SPI init, but DevOS also does full GT911 I2C config
  // which settles the chip on the bus. We only do the reset here — the ESPHome
  // GT911 component handles I2C config separately. Doing the reset after SPI init
  // avoids the GT911 interfering with TCA9555 I2C during bit-bang.
  this->touch_reset_();

  // Handle any enable pins (e.g. backlight) configured via YAML
  this->setup_enables_();

  // Start the ESP32-S3 RGB parallel display interface
  this->common_setup_();
}

void SQUiXLDisplay::touch_reset_() {
  // Replicate exact SQUiXL DevOS GT911 reset sequence from xgt911.cpp
  // INT pin HIGH during reset selects I2C address — must match DevOS behavior
  auto int_pin = static_cast<gpio_num_t>(this->touch_int_pin_);
  gpio_set_direction(int_pin, GPIO_MODE_OUTPUT);
  gpio_set_level(int_pin, 1);

  delay(5);

  this->expander_->pin_mode(this->exp_touch_rst_pin_, gpio::FLAG_OUTPUT);
  this->expander_->digital_write(this->exp_touch_rst_pin_, false);

  // Release reset
  this->expander_->digital_write(this->exp_touch_rst_pin_, true);
  delay(100);

  // Set INT as input
  gpio_set_direction(int_pin, GPIO_MODE_INPUT);

  ESP_LOGD(TAG, "GT911 touch controller reset complete");
  esp_task_wdt_reset();
}

void SQUiXLDisplay::hardware_reset_() {
  this->expander_->pin_mode(this->exp_rst_pin_, gpio::FLAG_OUTPUT);
  this->expander_->digital_write(this->exp_rst_pin_, true);
  delay(10);
  this->expander_->digital_write(this->exp_rst_pin_, false);
  delay(10);
  this->expander_->digital_write(this->exp_rst_pin_, true);
  delay(10);
}

void SQUiXLDisplay::write_9bit_(bool dc, uint8_t data) {
  // Clock out DC bit
  this->expander_->digital_write(this->exp_clk_pin_, false);
  this->expander_->digital_write(this->exp_mosi_pin_, dc);
  this->expander_->digital_write(this->exp_clk_pin_, true);

  // Clock out 8 data bits MSB first
  for (int i = 7; i >= 0; i--) {
    this->expander_->digital_write(this->exp_clk_pin_, false);
    this->expander_->digital_write(this->exp_mosi_pin_, (data >> i) & 0x01);
    this->expander_->digital_write(this->exp_clk_pin_, true);
  }
}

void SQUiXLDisplay::write_init_sequence_() {
  size_t index = 0;
  auto &seq = this->init_sequence_;

  while (index < seq.size()) {
    if (seq.size() - index < 2) {
      ESP_LOGE(TAG, "Malformed init sequence");
      this->mark_failed();
      return;
    }

    uint8_t cmd = seq[index++];
    uint8_t x = seq[index++];

    if (x == DELAY_FLAG) {
      ESP_LOGV(TAG, "Delay %dms", cmd);
      delay(cmd);
    } else {
      uint8_t num_args = x & 0x7F;
      if (seq.size() - index < num_args) {
        ESP_LOGE(TAG, "Malformed init sequence");
        this->mark_failed();
        return;
      }

      // CS low — start transaction
      this->expander_->digital_write(this->exp_cs_pin_, false);

      // Send command byte (DC = 0)
      this->write_9bit_(false, cmd);

      // Send data bytes (DC = 1)
      for (uint8_t i = 0; i < num_args; i++) {
        this->write_9bit_(true, seq[index++]);
      }

      // CS high — end transaction
      this->expander_->digital_write(this->exp_cs_pin_, true);

      ESP_LOGV(TAG, "Command 0x%02X, %d bytes", cmd, num_args);
    }

    // Feed watchdog — bit-bang init over I2C takes several seconds
    esp_task_wdt_reset();
  }

  this->init_sequence_.clear();
  delay(10);
}

void SQUiXLDisplay::dump_config() {
  mipi_rgb::MipiRgb::dump_config();
  ESP_LOGCONFIG(TAG, "  SQUiXL IO Expander Init");
  ESP_LOGCONFIG(TAG, "    LCD RST pin: %u", this->exp_rst_pin_);
  ESP_LOGCONFIG(TAG, "    LCD MOSI pin: %u", this->exp_mosi_pin_);
  ESP_LOGCONFIG(TAG, "    LCD CLK pin: %u", this->exp_clk_pin_);
  ESP_LOGCONFIG(TAG, "    LCD CS pin: %u", this->exp_cs_pin_);
  ESP_LOGCONFIG(TAG, "    Touch RST pin: %u", this->exp_touch_rst_pin_);
  ESP_LOGCONFIG(TAG, "    Touch INT pin: %u", this->touch_int_pin_);
}

}  // namespace squixl_display
}  // namespace esphome
#endif
