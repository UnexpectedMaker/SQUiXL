#ifndef SETTINGS_OPTIONS_H
#define SETTINGS_OPTIONS_H

#include "Arduino.h"
#include "utils/json.h"
#include "utils/json_conversions.h"
// #include "utilities/logging.h"
// #include "themes.h"
#include <vector>

struct wifi_station;
// struct theme;

class SettingsOptionBase
{

	public:
		// virtual methods
		virtual ~SettingsOptionBase() = default;
		virtual String generate_html(uint16_t index) { return ""; }

		enum Type
		{
			BASE,
			INT,
			BOOL,
			FLOAT,
			STRING,
			HEXCOLOR,
			INT_VECTOR,
			INT_RANGE,
			FLOAT_RANGE,
			WIFI_STATION,
			THEME
		};

		virtual Type getType() const
		{
			return BASE;
		}

		void convert_for_form_id(const char *input, char *output)
		{
			int i = 0;
			while (input[i] != '\0')
			{
				if (output[i] == ' ')
					output[i] = '_';
				else
					output[i] = tolower(input[i]);
				i++;
			}
			output[i] = '\0'; // Null-terminate the output string
		}

		void register_option();

		String fieldname = "";
		int group = -1;
		bool data_is_vector = false;
		bool req_full_width = false;
};

class SettingsOptionInt : public SettingsOptionBase
{
	public:
		SettingsOptionInt(int *val, int val_min, int val_max, bool wrap, int _group, const String &_fn) : setting_ref(val), value_min(val_min), value_max(val_max), value_wrap(wrap)
		{
			group = _group;
			fieldname = _fn;
			// register_option();
		}

		Type getType() const override
		{
			return INT;
		}

		int change(int dir);
		bool update(int val);
		int get();
		String get_str();
		String generate_html(uint16_t index);

	private:
		int *setting_ref = nullptr;
		int value_min = 0;
		int value_max = 0;
		bool value_wrap = false;
};

class SettingsOptionIntRange : public SettingsOptionBase
{
	public:
		SettingsOptionIntRange(int *val, int val_min, int val_max, int val_step, bool wrap, int _group, const String &_fn) : setting_ref(val), value_min(val_min), value_max(val_max), value_step(val_step), value_wrap(wrap)
		{
			group = _group;
			fieldname = _fn;

			// register_option();
		}

		Type getType() const override
		{
			return INT_RANGE;
		}

		int change(int dir);
		bool update(int val);
		int get();
		String get_str();
		String generate_html(uint16_t index);

		int get_min_value() { return value_min; }
		int get_max_value() { return value_max; }
		int get_step_value() { return value_step; }

		void log_data();

	private:
		int *setting_ref = nullptr;
		int value_min = 0;
		int value_max = 0;
		int value_step = 1;
		bool value_wrap = false;
};

class SettingsOptionFloatRange : public SettingsOptionBase
{
	public:
		SettingsOptionFloatRange(float *val, float val_min, float val_max, float val_step, bool wrap, int _group, const String &_fn) : setting_ref(val), value_min(val_min), value_max(val_max), value_step(val_step), value_wrap(wrap)
		{
			group = _group;
			fieldname = _fn;
			// register_option();
		}

		Type getType() const override
		{
			return FLOAT_RANGE;
		}

		float change(int dir);
		bool update(float val);
		float get();
		String get_str();
		String generate_html(uint16_t index);

		float get_min_value() { return value_min; }
		float get_max_value() { return value_max; }
		float get_step_value() { return value_step; }

	private:
		float *setting_ref = nullptr;
		float value_min = 0;
		float value_max = 0;
		float value_step = 1;
		bool value_wrap = false;
};

class SettingsOptionIntVector : public SettingsOptionBase
{
	public:
		SettingsOptionIntVector(std::vector<int> *val, int val_min, int val_max, int val_step, bool wrap, int _group, const String &_fn) : setting_ref(val), value_min(val_min), value_max(val_max), value_step(val_step), value_wrap(wrap)
		{
			group = _group;
			fieldname = _fn;
			data_is_vector = true;
			// register_option();
		}

		Type getType() const override
		{
			return INT_VECTOR;
		}

		int change(int index, int dir);
		bool update(int index, int val);
		int get(int index);
		String get_str(int index);
		String generate_html(uint16_t index);
		uint8_t vector_size();

	private:
		std::vector<int> *setting_ref = nullptr;
		int value_min = 0;
		int value_max = 0;
		int value_step = 1;
		bool value_wrap = false;

		//  String html = "";
};

class SettingsOptionFloat : public SettingsOptionBase
{
	public:
		SettingsOptionFloat(float *val, float val_min, float val_max, float val_step, bool wrap, int _group, const String &_fn) : setting_ref(val), value_min(val_min), value_max(val_max), value_step(val_step), value_wrap(wrap)
		{
			group = _group;
			fieldname = _fn;
			// register_option();
		}

		Type getType() const override
		{
			return FLOAT;
		}

		float change(int dir);
		bool update(float val);
		float get();
		String get_str();
		String generate_html(uint16_t index);

	private:
		float *setting_ref = nullptr;
		float value_min = 0;
		float value_max = 0;
		float value_step = 1;
		bool value_wrap = false;
};

class SettingsOptionBool : public SettingsOptionBase
{
	public:
		SettingsOptionBool(bool *val, int _group, const String &_fn, const String &_op1, const String &_op2) : setting_ref(val), option1(_op1), option2(_op2)
		{
			group = _group;
			fieldname = _fn;
			// register_option();
		}

		Type getType() const override
		{
			return BOOL;
		}

		bool change();
		bool update(bool val);
		bool get();
		String get_str();
		String get_fn();
		String get_op1();
		String get_op2();
		String generate_html(uint16_t index);

	private:
		bool *setting_ref = nullptr;

		String option1 = "";
		String option2 = "";
};

class SettingsOptionString : public SettingsOptionBase
{
	public:
		SettingsOptionString(String *val, int _group, const String &_fn, const String &pl = "", bool req = true) : setting_ref(val), required_field(req)
		{
			group = _group;
			fieldname = _fn;
			placeholder = pl;
			// register_option();
		}

		Type getType() const override
		{
			return STRING;
		}

		void change(String *val);
		bool update(String *val);
		String get();
		String generate_html(uint16_t index);

	private:
		String *setting_ref = nullptr;
		String placeholder = "";
		bool required_field = true;
};

class SettingsOptionColor565 : public SettingsOptionBase
{
	public:
		SettingsOptionColor565(uint16_t *val, int _group, const String &_fn, bool req = true) : setting_ref(val), required_field(req)
		{
			group = _group;
			fieldname = _fn;
		}

		Type getType() const override
		{
			return HEXCOLOR;
		}

		void change(uint16_t val);
		bool update(uint16_t val);
		uint16_t get();
		String generate_html(uint16_t index);

	private:
		uint16_t *setting_ref = nullptr;
		bool required_field = true;
};

class SettingsOptionWiFiStations : public SettingsOptionBase
{
	public:
		SettingsOptionWiFiStations(std::vector<wifi_station> *val, int _group, const String &_fn) : setting_ref(val)
		{
			group = _group;
			fieldname = _fn;
			data_is_vector = true;
			req_full_width = true;
			// register_option();
		}

		Type getType() const override
		{
			return WIFI_STATION;
		}

		bool update(int index, String _ssid, String _pass);
		void remove_if_empty();
		String get_ssid(int index);
		String get_pass(int index);
		void add_station(String ssid, String pass);
		String generate_html(uint16_t index);
		uint8_t vector_size();

	private:
		std::vector<wifi_station> *setting_ref = nullptr;
};

// class SettingsOptionTheme : public SettingsOptionBase
// {
// 	public:
// 		SettingsOptionTheme(std::vector<theme> *val, int _group, const String &_fn) : setting_ref(val)
// 		{
// 			group = _group;
// 			fieldname = _fn;
// 			data_is_vector = true;
// 			req_full_width = true;
// 			register_option(_group);
// 		}

// 		Type getType() const override
// 		{
// 			return THEME;
// 		}

// 		bool update(int index, String _name, int32_t _col_background_dull, uint32_t _col_background_bright, uint32_t _col_control_back, uint32_t _col_primary, uint32_t _col_secondary, uint32_t _col_low_intensity, uint32_t _col_warning, uint32_t _col_error, uint8_t _widget_style);
// 		void remove_if_empty();
// 		theme &get_theme(int index);
// 		// String get_pass(int index);
// 		void add_theme(String _name, int32_t _col_background_dull, uint32_t _col_background_bright, uint32_t _col_control_back, uint32_t _col_primary, uint32_t _col_secondary, uint32_t _col_low_intensity, uint32_t _col_warning, uint32_t _col_error, uint8_t _widget_style);
// 		String generate_html(uint16_t index);
// 		uint8_t vector_size();
// 		String rgbToHex(uint32_t rgbColor);
// 		String rgb565ToHex(uint16_t rgbColor);
// 		uint16_t hexToRgb565(String hexColor);

// 	private:
// 		std::vector<theme> *setting_ref = nullptr;
// };

/*
		uint32_t col_background_dull = 0;
		uint32_t col_background_bright = 0;
		uint32_t col_control_back = 0;
		uint32_t col_primary = 0;
		uint32_t col_secondary = 0;
		uint32_t col_low_intensity = 0;
		uint32_t col_warning = 0;
		uint32_t col_error = 0;
		uint8_t widget_style = 0;*/

#endif // SETTINGS_OPTIONS_H