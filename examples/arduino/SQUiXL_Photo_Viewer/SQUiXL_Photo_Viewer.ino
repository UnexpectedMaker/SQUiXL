/*
    SQUiXL Photo Viewer Example
    Copyright (c) 2025 Unexpected Maker
    https://squixl.io

    Requires a FAT formatted uSD card to be inserted into the uSD card slot on your SQUiXL,
    that contains some JPEG images named either with .JPG or .JPEG extension.
*/

#include "src/squixl_lite.h"
#include <JPEGDisplay.h>
#include <JPEGDisplay.inl>

// Sprite the JPG image gets loaded into
BB_SPI_LCD jpg_sprite;
// The JPEG Helper class
JPEGDisplay jd;
// The current image index
uint8_t current_image = 0;
// Storage of an arbitary number of image names found on the SD Card when initially scanning
// a std::vector is like an array, but it's dynamic so can shrink and grow at runtime.
std::vector<String> image_names;
// The next image swap time in millis()
unsigned long next_image_swap = 0;
// How quickly we can change images via touch
unsigned long next_image_swap_touch = 0;
// The time we show each image for before we load the next, n seconds
uint32_t time_between_image_swap = 20;

bool has_sdcard = false;

// Set the maximum amount of images to add to the list
uint8_t max_images = 20;

// Function to detect if the file is a JPG or not.
bool is_jpeg_filename(const char *name)
{
    if (!name || name[0] == '.')
    {
        return false; // hidden or invalid
    }

    size_t len = strlen(name);
    if (len < 4)
    {
        return false; // too short to be .jpg
    }

    // check for “.jpg”
    if (tolower(name[len - 4]) == '.' &&
        tolower(name[len - 3]) == 'j' &&
        tolower(name[len - 2]) == 'p' &&
        tolower(name[len - 1]) == 'g')
    {
        return true;
    }

    // check for “.jpeg”
    if (len >= 5 &&
        tolower(name[len - 5]) == '.' &&
        tolower(name[len - 4]) == 'j' &&
        tolower(name[len - 3]) == 'p' &&
        tolower(name[len - 2]) == 'e' &&
        tolower(name[len - 1]) == 'g')
    {
        return true;
    }

    return false;
}

// Function to scan the files in the root directory of the uSD card and if a JPG, add them to the image_names vector.
void scan_images_on_SD()
{
    File root = SD.open("/");
    if (!root)
    {
        Serial.println("Failed to open directory");
        return;
    }
    if (!root.isDirectory())
    {
        Serial.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while (file && image_names.size() < max_images)
    {
        if (!file.isDirectory())
        {
            // If the filename looks like a JPG/JPEG, add it to the vector
            if (is_jpeg_filename(file.name()))
            {
                String nm = "/" + String(file.name());
                image_names.push_back(nm);

                Serial.print("Added FILE: ");
                Serial.print(file.name());

                Serial.print(" SIZE: ");
                Serial.println(file.size());
            }
        }
        file = root.openNextFile();
    }

    // Print out the list of names - good for debugging.
    // Serial.println("File List:");
    // for (int i = 0; i < image_names.size(); i++)
    // {
    // 	Serial.println(image_names[i]);
    // }

    file.close();
}

// Swap the image being shown by loading the next JPG in the image_names vector
// and then alpha blending it onto the screen (fading it in)
void swap_image()
{
    // Load tge next JPG into the sprite
    if (jd.loadJPEG(&jpg_sprite, 0, 0, image_names[current_image].c_str()))
    {
        // If the load was successfull, fade it in.
        for (uint8_t alpha = 0; alpha < 32; alpha++)
        {
            squixl_lite.lcd.blendSprite(&jpg_sprite, &squixl_lite.lcd, &squixl_lite.lcd, alpha);
            delay(25);
        }
    }
    else
    {
        Serial.printf("Failed to load JPG %s\n", image_names[current_image].c_str());
    }

    // Increment the image index
    current_image++;
    if (current_image >= image_names.size())
        current_image = 0;
}

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

    // Switch the IOMUX to the uSD card slot. The uSD slot and the I2S Amplifier share the same IO via an IOMUX,
    // so only one can be active at a time, but they can be switched as required.
    has_sdcard = squixl_lite.mux_switch_to(MUX_STATE::MUX_SD);

    if (!has_sdcard)
    {
        // fade in error message and early exit
        squixl_lite.show_error("No uSD card found!", true);
        return;
    }

    // Initialise the sprite to load JPGs into, that we can re-use
    // It's loaded into PSRAM because it's bigger than what can fit in SRAM.
    jpg_sprite.createVirtual(480, 480, NULL, true);

    // Preload the image names into the vector
    scan_images_on_SD();

    // Quick pause to allow the screen to be ready
    delay(100);

    next_image_swap = millis();
    // Allow the first image to load immediately
    swap_image();
}

void loop()
{
    if (!has_sdcard)
        return;

    // Check for the next image swap time
    if (millis() - next_image_swap > (time_between_image_swap * 1000))
    {
        next_image_swap = millis();
        swap_image();
    }

    // If a touch was detected, it sets x,y and returns true,
    // Touching the screen will force the next image swap and reset the timer
    uint16_t x = 0;
    uint16_t y = 0;
    if (squixl_lite.process_touch_lite(&x, &y))
    {
        // Only allow touch to switch images every second
        if (millis() - next_image_swap_touch > 1000)
        {
            next_image_swap_touch = millis();
            Serial.printf("Touched (%d,%d)\n", x, y);
            swap_image();
        }
    }

} /* loop() */
