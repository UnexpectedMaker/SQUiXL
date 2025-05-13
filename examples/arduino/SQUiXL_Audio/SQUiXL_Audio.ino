/*
    SQUiXL Audio Example
    Copyright (c) 2025 Unexpected Maker
    https://squixl.io
*/

#include "src/squixl_lite.h"

uint16_t last_x = 0;
uint16_t last_y = 0;

bool played_hello = false;

// How quickly we can make beeps via touch
unsigned long next_beep_touch = 0;

// Sprite for a second screen buffer
BB_SPI_LCD screen_sprite;

void setup()
{
    // Set PWM for backlight chage pump IC
    pinMode(BL_PWM, OUTPUT);
    ledcAttach(BL_PWM, 6000, LEDC_TIMER_12_BIT);

    Serial.begin(115200);
    Serial.setDebugOutput(true); // sends all log_e(), log_i() messages to USB HW CDC

    // Initialise the I2C bus on SQUiXL - used to bitband the screen initialisation
    Wire.begin(1, 2);        // UM square
    Wire.setBufferSize(256); // IMPORTANT: GT911 needs this
    Wire.setClock(400000);   // Make the I2C bus fast!

    // Initialise the screen
    squixl_lite.init();

    squixl_lite.lcd.fillScreen(TFT_BLUE);

    // Switch the IOMUX to the I2S Audio Amplifier. The uSD slot and the I2S Amplifier share the same IO via an IOMUX,
    // so only one can be active at a time, but they can be switched as required.
    squixl_lite.mux_switch_to(MUX_STATE::MUX_I2S);

    // Initialise the sprite in PSRAM because it's bigger than what can fit in SRAM.
    screen_sprite.createVirtual(480, 480, NULL, true);

    // Select the font we want to use for the screen, set the font colour (white with transparent back) and set the cursor and print some text
    screen_sprite.fillScreen(TFT_BLUE);
    screen_sprite.setFreeFont(UbuntuMono_R[2]);
    screen_sprite.setTextColor(TFT_WHITE, -1);
    screen_sprite.setCursor(50, 30);
    screen_sprite.print("Hello! Touch the screen to play a tone!");
    squixl_lite.lcd.drawSprite(0, 0, &screen_sprite, 1.0, -1);

    delay(500);
}

void loop()
{
    // Make SQUiXL play the included "hello" audio
    // We don't want to do this in setup() as it can hold up execution of the update loop
    if (!played_hello)
    {
        played_hello = true;

        audio.set_volume(15);
        audio.play_wav("hello");
    }

    // We call update on the audio class to allow any playing audio to process, and any finished audio to close
    if (squixl_lite.mux_check_state(MUX_STATE::MUX_I2S))
        audio.update();

    // If a touch was detected, it sets x,y and returns true,
    // Touching the screen will play a beep based on the spot you touched
    uint16_t x = 0;
    uint16_t y = 0;
    if (squixl_lite.process_touch_lite(&x, &y))
    {
        // Only allow touch to switch images every 1/4 second (250 ms)
        if (millis() - next_beep_touch > 250)
        {
            next_beep_touch = millis();
            Serial.printf("Touched (%d,%d)\n", x, y);
            // Just make a random 565 color - nasty but effective ;)
            uint16_t color = random(65000);
            // Draw a line from the last touch to the new one and then place a small ciecle there.
            screen_sprite.drawLine(x, y, last_x, last_y, color);
            screen_sprite.fillCircle(x, y, 9, color);
            squixl_lite.lcd.drawSprite(0, 0, &screen_sprite, 1.0, -1);
            audio.play_tone((float)x / 50.0, (float)y / 50.0);

            last_x = x;
            last_y = y;
        }
    }

} /* loop() */
