/*
    LVGL Example for SQUiXL
    Copyright (c) 2025 Unexpected Maker
    https://squixl.io
*/

#include "squixl.h"
#include "utils/squixl_logo_blue.h"
#include <lvgl.h>
#include <bb_spi_lcd.h>
BB_SPI_LCD lcd;
uint16_t dma_buf[512];
const lv_point_t points_array[3] = {{53, 30}, {160, 30}, {261, 30}};
lv_obj_t *label, *msg_label, *label_a, *label_b, *label_c, *slider_label, *battery_label;
lv_obj_t *btn1, *btn2, *btn3;
lv_obj_t *scr, *slider1;

lv_obj_t *squixl_logo; // squixl_logo_blue

#define DRAW_BUF_SIZE(w, h) ((w * h) / 10 * sizeof(uint16_t))
uint16_t *draw_buf;

#if LV_USE_LOG != 0
void my_print(lv_log_level_t level, const char *buf)
{
    LV_UNUSED(level);
    Serial.println(buf);
    Serial.flush();
}
#endif

unsigned long next_battery_check = 0;
unsigned long next_lvgl_update = 0;

/* LVGL calls it when a rendered image needs to copied to the display*/
/* Arduino devices are almost exclusively little-endian machines, but SPI */
/* LCDs are big endian, so we need to swap the byte order */
void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    /*Copy `px map` to the `area`*/
    uint32_t w = lv_area_get_width(area);
    uint32_t h = lv_area_get_height(area);
    uint16_t *s = (uint16_t *)px_map;
    lcd.setAddrWindow(area->x1, area->y1, w, h);
    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
        {
            dma_buf[x] = __builtin_bswap16(s[x]); // convert to big-endian
        }
        lcd.pushPixels(dma_buf, w, DRAW_TO_LCD | DRAW_WITH_DMA);
        s += w;
    }

    /*tell LVGL you are ready*/
    lv_display_flush_ready(disp);
} /* my_disp_flush */

static void event_handler_a(lv_event_t *e)
{
    lv_label_set_text(msg_label, "You pressed A");
    audio.play_menu_beep(3);
}

static void event_handler_b(lv_event_t *e)
{
    lv_label_set_text(msg_label, "You pressed B");
    audio.play_menu_beep(5);
}

static void event_handler_c(lv_event_t *e)
{
    lv_label_set_text(msg_label, "You pressed C");
    audio.play_menu_beep(7);
}

/*use Arduinos millis() as tick source*/
static uint32_t my_tick(void)
{
    return millis();
}

void my_captouch_read(lv_indev_t *indev, lv_indev_data_t *data)
{

    uint16_t pts[5][4];
    uint8_t n = xtouch.readPoints(pts);

    if (n > 0)
    {
        data->state = LV_INDEV_STATE_PRESSED;
        data->point.x = pts[0][0];
        data->point.y = pts[0][1];
    }
    else
    {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

static void slider_event_cb(lv_event_t *e)
{
    lv_obj_t *slider = lv_event_get_target_obj(e);
    char buf[8];
    lv_snprintf(buf, sizeof(buf), "%d C", (int)lv_slider_get_value(slider));
    lv_label_set_text(slider_label, buf);
    // Serial.println(buf);
}

void setup()
{
    Serial.begin(115200);
    Serial.setDebugOutput(true); // sends all log_e(), log_i() messages to USB HW CDC
    Serial.setTxTimeoutMs(0);    // sets no timeout when trying to write to USB HW CDC

    Wire.begin(1, 2);        // UM square
    Wire.setBufferSize(256); // IMPORTANT: GT911 needs this
    Wire.setClock(400000);

    squixl.init();
    haptics.init();

    // Switch the IOMUX to the I2S Amplifier for audio output
    squixl.mux_switch_to(MUX_STATE::MUX_I2S); // set to I2S
    audio.set_volume(settings.config.volume);

    rtc.init();
    battery.init();

    /* Initilise Touch via GT911 */
    xtouch.begin(TP_INT, TP_RST);

    String LVGL_Arduino = "Hello Arduino! ";
    LVGL_Arduino += String('V') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();

    Serial.println(LVGL_Arduino);

    lv_init();

    /*Set a tick source so that LVGL will know how much time elapsed. */
    lv_tick_set_cb(my_tick);

    /* register print function for debugging */
#if LV_USE_LOG != 0
    lv_log_register_print_cb(my_print);
#endif

    lv_display_t *disp;
    int w, h, iSize;

    // Now we init the display via the ESP32-S3 RGB Peripheral
    lcd.begin(DISPLAY_UM_480x480);
    w = lcd.width();
    h = lcd.height();
    iSize = DRAW_BUF_SIZE(w, h);
    draw_buf = (uint16_t *)malloc(iSize);
    disp = lv_display_create(w, h);
    lv_display_set_flush_cb(disp, my_disp_flush);
    lv_display_set_buffers(disp, draw_buf, NULL, iSize, LV_DISPLAY_RENDER_MODE_PARTIAL);

    // Setup the touch driver
    lv_indev_t *indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER); /*Touchpad should have POINTER type*/
    lv_indev_set_read_cb(indev, my_captouch_read);

    // create an lvgl screen
    scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x000000), 0); // set background color to black
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    // display the SQUiXL logo
    LV_IMAGE_DECLARE(squixl_logo_blue);
    squixl_logo = lv_image_create(lv_screen_active());
    lv_image_set_src(squixl_logo, &squixl_logo_blue);
    lv_obj_align(squixl_logo, LV_ALIGN_TOP_MID, 0, 20);

    // display the project title
    label = lv_label_create(lv_screen_active());
    lv_label_set_text(label, "LVGL Basic UI Demo");
    lv_obj_set_style_text_color(label, lv_color_make(0xff, 0xff, 0xff), 0);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 95);

    // Buttons and label
    msg_label = lv_label_create(lv_screen_active());
    lv_label_set_text(msg_label, "Press a button");
    lv_obj_set_style_text_color(msg_label, lv_color_make(0xff, 0xff, 0xff), 0);
    lv_obj_align(msg_label, LV_ALIGN_CENTER, 0, -50);

    btn1 = lv_button_create(lv_screen_active());
    lv_obj_set_size(btn1, 80, 60);
    lv_obj_set_pos(btn1, 80, 220);
    lv_obj_add_event_cb(btn1, event_handler_a, LV_EVENT_PRESSED, NULL);
    lv_obj_remove_flag(btn1, LV_OBJ_FLAG_PRESS_LOCK);
    lv_obj_set_style_bg_color(btn1, lv_color_make(0, 0xff, 0), LV_PART_MAIN); // green
    label_a = lv_label_create(btn1);
    lv_label_set_text(label_a, "A");
    lv_obj_center(label_a);
    lv_obj_set_style_text_color(label_a, lv_color_make(0, 0, 0), 0);

    btn2 = lv_button_create(lv_screen_active());
    lv_obj_set_size(btn2, 80, 60);
    lv_obj_set_pos(btn2, 200, 220);
    lv_obj_add_event_cb(btn2, event_handler_b, LV_EVENT_PRESSED, NULL);
    lv_obj_remove_flag(btn2, LV_OBJ_FLAG_PRESS_LOCK);
    lv_obj_set_style_bg_color(btn2, lv_color_make(0xff, 0xff, 0), LV_PART_MAIN); // yellow
    label_b = lv_label_create(btn2);
    lv_label_set_text(label_b, "B");
    lv_obj_center(label_b);
    lv_obj_set_style_text_color(label_b, lv_color_make(0, 0, 0), 0);

    btn3 = lv_button_create(lv_screen_active());
    lv_obj_set_size(btn3, 80, 60);
    lv_obj_set_pos(btn3, 320, 220);
    lv_obj_add_event_cb(btn3, event_handler_c, LV_EVENT_PRESSED, NULL);
    lv_obj_remove_flag(btn3, LV_OBJ_FLAG_PRESS_LOCK);
    lv_obj_set_style_bg_color(btn3, lv_color_make(0, 0, 0xff), LV_PART_MAIN); // blue
    label_c = lv_label_create(btn3);
    lv_label_set_text(label_c, "C");
    lv_obj_center(label_c);
    lv_obj_set_style_text_color(label_c, lv_color_make(0, 0, 0), 0);

    // Slider & Label
    slider_label = lv_label_create(lv_screen_active());
    lv_label_set_text(slider_label, "Slider Value");
    lv_obj_set_style_text_color(slider_label, lv_color_make(0xff, 0xff, 0xff), 0);
    lv_obj_align(slider_label, LV_ALIGN_BOTTOM_MID, 0, -150);

    slider1 = lv_slider_create(lv_screen_active());
    lv_obj_align(slider1, LV_ALIGN_BOTTOM_MID, 0, -120);
    lv_obj_set_height(slider1, 10);
    lv_slider_set_mode(slider1, LV_SLIDER_MODE_RANGE);
    lv_slider_set_range(slider1, 10, 35); // min/max value
    lv_slider_set_value(slider1, 20, LV_ANIM_OFF);
    lv_obj_add_event_cb(slider1, slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_refresh_ext_draw_size(slider1);

    // Battery
    battery_label = lv_label_create(lv_screen_active());
    lv_label_set_text(battery_label, "Battery Stats");
    lv_obj_set_style_text_color(battery_label, lv_color_make(0x00, 0xff, 0x00), 0);
    lv_obj_align(battery_label, LV_ALIGN_BOTTOM_MID, 0, -20);

    Serial.println("Setup done");

    // Buzz the haptics on first display
    haptics.play_trigger(Triggers::STARTUP);
}

void loop()
{
    // let the GUI do its work
    if (millis() - next_lvgl_update > 5)
    {
        next_lvgl_update = millis();
        lv_timer_handler();
    }

    // Show the battery voltage every 10 seconds
    if (millis() - next_battery_check > 10000)
    {
        next_battery_check = millis();
        char buf[12];
        snprintf(buf, sizeof(buf), "%0.1fV %d%%", battery.get_voltage(true), (int)battery.get_percent(true));
        lv_label_set_text(battery_label, buf);
    }

    // Process the audio loop
    audio.update();
}