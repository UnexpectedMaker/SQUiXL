#pragma once

#include "ui/ui_element.h"
#include "fonts/ubuntu_mono_all_r.h"
#include <map>
#include <vector>
#include <string>

class ui_control_base;

class ui_window : public ui_element
{
	public:
		using CallbackFunction = void (*)();

		void create(int16_t pos_x, int16_t pos_y, int16_t width, int16_t height, int16_t color, uint8_t transparecy, uint8_t blur_count, const char *title);

		// void show(bool fade_in = false);

		// void move(int16_t pos_x, int16_t pos_y, bool fade_in = false);

		// void set_title(const char *title = nullptr, TEXT_ALIGN align = ALIGN_CENTER, GFXfont *font = UbuntuMono_R[0]);

		void set_title_alignment(TEXT_ALIGN align);
		// void set_title_font(GFXfont *font);
		// void set_title_alignment(TEXT_ALIGN align = ALIGN_CENTER);

		void draw_window_heading();

		bool redraw(uint8_t fade_amount) override;

		bool process_touch(touch_event_t touch_event) override;

	protected:
		int16_t _adj_x;			// alignment adjusted draw pos x
		int16_t _adj_y;			// alignment adjusted draw pos y
		uint8_t padding_l = 10; // left padding for window content
		uint8_t padding_t = 30; // top padding for window content
		uint8_t padding_r = 10; // right padding for window content
		uint8_t padding_b = 10; // bottom padding for window content

		uint16_t _text_width, _text_height;

		const GFXfont *_font = UbuntuMono_R[1];

		bool calculate_text_size(bool forced = false);
};