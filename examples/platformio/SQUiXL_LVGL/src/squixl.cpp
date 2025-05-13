#include "squixl.h"

void SQUiXL::init()
{
    // Set PWM for backlight chage pump IC
    pinMode(BL_PWM, OUTPUT);
    ledcAttach(BL_PWM, 6000, LEDC_TIMER_12_BIT);

    /* Initialise LCA9555 IO Expander */
    if (ioex.begin(0x20))
    {
        // LCD Reset Line
        ioex.pin_mode(LCD_RST, OUTPUT, HIGH);

        // Screen backlight EN
        ioex.pin_mode(BL_EN, OUTPUT, HIGH);

        // // Screen soft power EN
        // ioex.pin_mode(SOFT_PWR, OUTPUT, LOW);

        // Haptic EN
        ioex.pin_mode(HAPTICS_EN, OUTPUT, HIGH);

        ioex.pin_mode(VBUS_SENSE, INPUT);

        // IO MUX - EN is Active LOW, so start it off
        ioex.pin_mode(MUX_EN, OUTPUT, HIGH);
        // IO MUX - Set default to I2S - LOW is SD
        ioex.pin_mode(MUX_SEL, OUTPUT, HIGH);

        Serial.println("LCA9555 is Good!");
    }
    else
    {
        Serial.println("LCA9555 Init Failed");
        return;
    }

    // First we need to init the LCD via IOEX bitbanging (Not in current PCB version)
    screen_init_spi_bitbanged(st7701s_init_commands);
}

void SQUiXL::screen_init_spi_bitbanged(const uint8_t *data)
{
    Serial.print("Bit banging LCD init... ");
    unsigned long timer = millis();
    const uint8_t MOSI = 2; // P02
    const uint8_t CLK = 3;  // P02
    const uint8_t CS = 4;   // P03

    // Configure pins as output
    ioex.pin_mode(MOSI, OUTPUT);
    ioex.pin_mode(CLK, OUTPUT);
    ioex.pin_mode(CS, OUTPUT);

    // Set initial pin states
    ioex.write(CS, HIGH);
    ioex.write(CLK, LOW);

    ioex.write(MOSI, LOW);

    int bytes_written = 0;

    while (*data)
    {
        uint8_t len = *data++;
        if (len == LCD_DELAY)
        {
            delay(*data++);
            continue;
        }
        uint8_t cmd = *data++;

        // Start SPI transaction
        ioex.write(CS, LOW);

        // Clock DC bit for command
        ioex.write(CLK, LOW);
        ioex.write(MOSI, 0);
        // delayMicroseconds(1);
        ioex.write(CLK, HIGH);

        // Send command
        for (int i = 7; i >= 0; i--)
        {
            ioex.write(CLK, LOW);
            ioex.write(MOSI, (cmd >> i) & 0x01);
            // delayMicroseconds(1);
            ioex.write(CLK, HIGH);
            // delayMicroseconds(1);
        }

        // Send data bytes
        for (uint8_t i = 0; i < len - 1; i++)
        {
            uint8_t byte = *data++;

            // Clock DC bit for data
            ioex.write(CLK, LOW);
            ioex.write(MOSI, 1);
            // delayMicroseconds(1);
            ioex.write(CLK, HIGH);

            for (int j = 7; j >= 0; j--)
            {
                ioex.write(CLK, LOW);
                ioex.write(MOSI, (byte >> j) & 0x01);
                // delayMicroseconds(1);
                ioex.write(CLK, HIGH);
                // delayMicroseconds(1);
            }
        }

        // End SPI transaction
        ioex.write(CS, HIGH);
    }

    Serial.printf("Done in %f seconds\n", (millis() - timer));
}

void SQUiXL::set_backlight_level(uint16_t pwm_level)
{
    ledcWrite(BL_PWM, pwm_level);
}

/*
IO MUX Stuff
*/
void SQUiXL::mux_toggle()
{
    if (millis() - next_mux_switch < 500)
        return;

    next_mux_switch = millis();

    if (_current_mux_state == MUX_STATE::MUX_I2S)
        mux_switch_to(MUX_STATE::MUX_SD);
    else
        mux_switch_to(MUX_STATE::MUX_I2S);
}

bool SQUiXL::mux_check_state(MUX_STATE new_state)
{
    return (_current_mux_state == new_state);
}

bool SQUiXL::mux_switch_to(MUX_STATE new_state)
{
    if (new_state == MUX_STATE::MUX_I2S && _current_mux_state == MUX_STATE::MUX_I2S)
    {
        Serial.println("I2S Already Selected!");
        return false;
    }
    else if (new_state == MUX_STATE::MUX_SD && _current_mux_state == MUX_STATE::MUX_SD)
    {
        Serial.println("SD Already Selected!");
        return false;
    }
    else if (new_state == MUX_STATE::MUX_OFF && _current_mux_state == MUX_STATE::MUX_OFF)
    {
        Serial.println("MUX Already Off");
        return false;
    }

    // We are switching, so shutdown whatever is current
    if (_current_mux_state == MUX_STATE::MUX_I2S)
    {
        audio.stop();
        Serial.println("Stopped I2S Peripheral");
    }
    else if (_current_mux_state == MUX_STATE::MUX_SD)
    {
        SD.end();
        SPI.end();
        Serial.println("Stopped SD");
    }
    else if (_current_mux_state == MUX_STATE::MUX_OFF)
    {
        // Re-enable the IO MNUX
        Serial.println("Enabled MUX");
        ioex.write(MUX_EN, LOW);
    }

    delay(50);

    // Now do teh switch
    if (new_state == MUX_STATE::MUX_I2S)
    {
        ioex.write(MUX_SEL, HIGH); // HIGH is I2S, LOW is SD
        delayMicroseconds(1);
        audio.setup(MUX_D3, MUX_D4, MUX_D2, MUX_D1);
        delay(10);
        Serial.println("Switched to I2S via MUX");
        _current_mux_state = MUX_STATE::MUX_I2S;
    }
    else if (new_state == MUX_STATE::MUX_SD)
    {
        ioex.write(MUX_SEL, LOW); // HIGH is I2S, LOW is SD
        delay(10);

        Serial.println("Switched to SD via MUX");

        SPI.begin(MUX_D3, MUX_D1, MUX_D4, MUX_D2);
        if (!SD.begin(MUX_D2))
        {

            Serial.println("Card Mount Failed");
            return false;
        }
        uint8_t cardType = SD.cardType();

        if (cardType == CARD_NONE)
        {
            Serial.println("No SD card attached");
            return false;
        }

        Serial.print("SD Card Type: ");
        if (cardType == CARD_MMC)
        {
            Serial.println("MMC");
        }
        else if (cardType == CARD_SD)
        {
            Serial.println("SDSC");
        }
        else if (cardType == CARD_SDHC)
        {
            Serial.println("SDHC");
        }
        else
        {
            Serial.println("UNKNOWN");
        }

        uint64_t cardSize = SD.cardSize() / (1024 * 1024);
        Serial.printf("SD Card Size: %lluMB\n", cardSize);

        // Setup SD card stuff here

        _current_mux_state = MUX_STATE::MUX_SD;
    }
    else if (new_state == MUX_STATE::MUX_OFF)
    {
        // Disable the IO MNUX
        ioex.write(MUX_EN, HIGH);
        Serial.println("Disabled MUX");
    }

    return true;
}

bool SQUiXL::vbus_present()
{
    return (is_5V_detected);
}

/// @brief Detect if 5V as gone from NO to YES. Look complicated, but we dont want to check a change in both ways, just a change from NO to YES.
/// @return if 5V was not present and now is.
bool SQUiXL::vbus_changed()
{
    bool detected = false;
    bool vbus = ioex.read(VBUS_SENSE);
    if (vbus && vbus != is_5V_detected)
        detected = true;

    is_5V_detected = vbus;
    return detected;
}

SQUiXL squixl;