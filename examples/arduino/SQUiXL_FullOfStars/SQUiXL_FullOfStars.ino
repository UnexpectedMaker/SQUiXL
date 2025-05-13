/*
	SQUiXL Full Of Stars Example
	Copyright (c) 2025 Unexpected Maker
	https://squixl.io
*/

#include "src/squixl_lite.h"

// Sprite for a second screen buffer
BB_SPI_LCD screen_sprite;

struct Star
{
		float x, y, z, pz;
};

const int STAR_COUNT = 150;
Star stars[STAR_COUNT];
// tweak for faster/slower “flight”
float SPEED = 8.0;

void setup_starfield()
{
	int w2 = 240;
	int h2 = 240;
	for (int i = 0; i < STAR_COUNT; i++)
	{
		stars[i].x = random(-w2, w2);
		stars[i].y = random(-h2, h2);
		stars[i].z = random(480);
		stars[i].pz = stars[i].z;
	}
}

void draw_starfield()
{
    screen_sprite.fillScreen(0);

    const int   w2          = 480 / 2;
    const int   h2          = 480 / 2;
    const float maxD        = 480.0f;
    const bool  glowing     = (SPEED > 20.0f);
    const float blueThresh  = 0.6f;     // only stars with b_f > blueThresh go blue

    for (int i = 0; i < STAR_COUNT; i++)
    {
        Star &s = stars[i];

        // move star toward us
        s.z -= SPEED;
        if (s.z < 1) {
            s.x  = random(-w2, w2);
            s.y  = random(-h2, h2);
            s.z  = maxD;
            s.pz = s.z;
        }

        // project to 2D screen coords
        int sx = int(s.x / s.z  * w2 + w2);
        int sy = int(s.y / s.z  * h2 + h2);
        int px = int(s.x / s.pz * w2 + w2);
        int py = int(s.y / s.pz * h2 + h2);

        // brightness from 0.0 (far) → 1.0 (close)
        float b_f = 1.0f - (s.z / maxD);
        b_f = max(0.0f, b_f);

        uint8_t b8 = b_f * 255;  // 8-bit brightness
        uint16_t col;

        if (glowing && b_f > blueThresh) {
            // only these close stars get the blue glow
            uint8_t r8 = b8 >> 2;  // ~25% red
            uint8_t g8 = b8 >> 1;  // ~50% green
            uint8_t bl = b8;       // 100% blue
            col = ((r8 & 0xF8) << 8)
                | ((g8 & 0xFC) << 3)
                |  (bl  >> 3);
        } else {
            // normal grey for all others
            col = ((b8 & 0xF8) << 8)
                | ((b8 & 0xFC) << 3)
                |  (b8 >> 3);
        }

        screen_sprite.drawLine(px, py, sx, sy, col);
        s.pz = s.z;
    }

    squixl_lite.lcd.drawSprite(0, 0, &screen_sprite, 1.0, -1);
}

void setup()
{
	// Set PWM for backlight chage pump IC
	pinMode(BL_PWM, OUTPUT);
	ledcAttach(BL_PWM, 6000, LEDC_TIMER_12_BIT);

	Serial.begin(115200);
	Serial.setDebugOutput(true); // sends all log_e(), log_i() messages to USB HW CDC

	// Initialise the I2C bus on SQUiXL - used to bitband the screen initialisation
	Wire.begin(1, 2);		 // UM square
	Wire.setBufferSize(256); // IMPORTANT: GT911 needs this
	Wire.setClock(400000);	 // Make the I2C bus fast!

	// Initialise the screen
	squixl_lite.init();

	// Set the SPI frequency for the screen to 16Mhz to rsqueeze some extra performance for more stars
	RGBChangeFreq(16000000);

	squixl_lite.lcd.fillScreen(0);

	// Initialise the sprite in PSRAM because it's bigger than what can fit in SRAM.
	screen_sprite.createVirtual(480, 480, NULL, true);

	setup_starfield();

	delay(500);
}

void loop()
{
	uint16_t x = 0;
	uint16_t y = 0;
	// We don't care about the touch co-ordinates, we just want to know if the screen is being touched or not.
	bool touched = squixl_lite.process_touch_lite(&x, &y);

    // Now ramp the SPEED up or down based on if the user is trouching the screen or not.
	if (touched)
		SPEED = constrain(SPEED + 5.0, 8.0, 32.0);
	else
		SPEED = constrain(SPEED - 5.0, 8.0, 32.0);

	draw_starfield();
	delay(30); // adjust to control framerate

} /* loop() */
