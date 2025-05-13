#pragma once

#include "ui/dashboard/ui_gauge.h"

class ui_gauge_bme280 : public ui_gauge
{
	public:
		bool redraw(uint8_t fade_amount) override;
		bool process_touch(touch_event_t touch_event) override;

	protected:
};