#pragma once

#include "wifi_common.h"
#include <Arduino.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include "settings/settings.h"

class WebServer
{

	public:
		WebServer() : web_server(80) {}

		void start();
		void start_callback(bool success, const String &response);
		void stop(bool restart);
		void process();
		bool is_running();
		void process_version(bool success, const String &response);

		static String processor(const String &var);

		static String generate_settings_html(int group_id);
		static String generate_themes_html(int group_id);

		SettingsOptionBase *get_obj_from_id(String id);

	private:
		bool _running = false;
		AsyncWebServer web_server;
		DNSServer dns_server;
};

extern WebServer webserver;