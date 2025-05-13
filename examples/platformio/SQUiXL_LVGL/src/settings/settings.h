#pragma once

#include <vector>
#include <map>
#include "utils/json.h"
#include "utils/json_conversions.h"

using json = nlohmann::json;

// Save data struct
struct Config
{
		int ver = 1;
		bool first_time = true;

		bool wifi_start = false;
		String wifi_ssid = "";
		String wifi_pass = "";
		int wifi_tx_power = 44;

		String mdns_name = "SQUiXL";

		String country = "";
		String city = "";
		int16_t utc_offset = 999;

		// Time
		bool time_24hour = false;
		bool time_dateformat = false; // False - DMY, True - MDY

		float volume = 15.0;

		json last_saved_data;
};

class Settings
{

	public:
		Settings(void)
		{
		}

		Config config;

		bool load();
		bool save(bool force);
		bool create();
		void print_file();
		bool has_wifi_creds(void);
		bool has_country_set(void);
		void update_wifi_credentials(String ssid, String pass);

	private:
		static constexpr const char *filename = "/settings.json";
		static constexpr const char *tmp_filename = "/tmp_settings.json";

		unsigned long max_time_between_saves = 30000; // every 30 seconds
		unsigned long last_save_time = 0;
};

extern Settings settings;
