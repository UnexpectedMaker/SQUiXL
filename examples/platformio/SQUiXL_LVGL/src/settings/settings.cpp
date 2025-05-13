#include "Arduino.h"
#include "settings.h"
#include <LittleFS.h>

using json = nlohmann::json;

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Config, wifi_tx_power, wifi_ssid, wifi_pass, mdns_name, city, country, utc_offset, time_24hour, time_dateformat, volume);

/**
 * @brief Checks to see if there are WiFi credentials stores in the user settings
 *
 * @return true credentials are not empty strings
 * @return false credentials are empty strings
 */
bool Settings::has_wifi_creds(void)
{
	return !config.wifi_ssid.isEmpty() && !config.wifi_pass.isEmpty();
}

bool Settings::has_country_set(void) { return !config.country.isEmpty(); }

/**
 * @brief Update the users WiFi credentials in the settings struct
 *
 * @param ssid
 * @param pass
 */
void Settings::update_wifi_credentials(String ssid, String pass)
{
	config.wifi_ssid = ssid;
	config.wifi_pass = pass;
	save(true);
}

/**
 * @brief Load the user settings from the FLASH FS and deserialise them from JSON back into the Config struct
 *
 * @return true
 * @return false
 */
bool Settings::load()
{
	Serial.println("Loading settings");

	File file = LittleFS.open(filename);
	if (!file || file.isDirectory() || file.size() == 0)
	{
		// No data on the flash chip, so create new data
		file.close();
		create();
		// log_to_nvs("load_status", "no file");
		return false;
	}

	std::vector<char> _data(file.size());
	size_t data_bytes_read = file.readBytes(_data.data(), _data.size());
	if (data_bytes_read != _data.size())
	{
		// Reading failed
		String log = "bad read " + String(file.size()) + " " + String((int)data_bytes_read);
		// log_to_nvs("load_status", log.c_str());
		file.close();
		create();
		return false;
	}

	try
	{
		json json_data = json::parse(_data);

		// Convert json to struct
		config = json_data.get<Config>();

		// Store loaded data for comparison on next save
		config.last_saved_data.swap(json_data);
	}
	catch (json::exception &e)
	{
		Serial.println("Settings parse error:");
		Serial.println(e.what());
		file.close();
		create();
		// log_to_nvs("load_status", "bad json parse");
		return false;
	}

	file.close();

	Serial.println("Settings: Load complete!");

	return true;
}

/**
 * @brief Serialise the cConfig struct into JSON and save to the FLASH FS
 * Only check for save every 5 mins, and then only save if the data has changed
 *
 * We only want to save data when it's changed because we dont want to wear out the Flash.
 *
 * @param force for the save regardless of time, but again, only if the data has changed
 * @return true
 * @return false
 */
bool Settings::save(bool force)
{
	// We only want to attempt  save every 1 min unless it's a forced save.
	if (!force && millis() - last_save_time < max_time_between_saves)
		return false;

	// Implicitly convert struct to json
	json data = config;

	// If the data is the same as the last data we saved, bail out
	if (data == config.last_saved_data)
	{
		last_save_time = millis();
		return false;
	}

	std::string serializedObject = data.dump();

	File file = LittleFS.open(tmp_filename, FILE_WRITE);
	if (!file)
	{
		Serial.println("Failed to write to settings file");
		// log_to_nvs("save_status", "failed to open for write");
		return false;
	}

	file.print(serializedObject.c_str());
	// log_to_nvs("save_status", "data written");

	file.close();
	// log_to_nvs("save_status", "file closed");

	LittleFS.rename(tmp_filename, filename);
	// log_to_nvs("save_status", "file renamed");

	Serial.println("Settings SAVE: Saved!");

	// Store last saved data for comparison on next save
	config.last_saved_data.swap(data);

	last_save_time = millis();
	return true;
}

/**
 * @brief Create a new set of save data, either because this is the very first save, or because the load failed due to FS corruption,
 * or malformed JSON data that could not be deserialised.
 *
 * Once created, the data is automatically saved to flash.
 *
 * @return true
 * @return false
 */
bool Settings::create()
{
	Serial.println("Settings CREATE: Creating new data...");

	config = {};

	save(true);

	return true;
}

void Settings::print_file()
{
	File file = LittleFS.open(filename);
	std::vector<char> _data(file.size());
	size_t data_bytes_read = file.readBytes(_data.data(), _data.size());

	Serial.println("Settings JSON");
	for (char c : _data)
	{
		Serial.print(c);
	}
	Serial.println();

	file.close();
}

Settings settings;