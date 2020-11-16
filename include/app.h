#pragma once
#include <application.h>

class AppClass : public ApplicationClass
{
	static const int beeperPin{4};
	static const int receivePin{12};
	static const int receiveRefresh{20}; // Try to receive every receiveRefresh ms.

	static String serverURL;
	static bool sound;

	void _loadAppConfig(file_t& file) override;
	void _saveAppConfig(file_t& file) override;
	bool _extraConfigReadJson(JsonObject& json) override;
	void _extraConfigWriteJson(JsonObject& json) override;
	static void receiveRF();
	static void httpPost(unsigned long value);

public:
	static const String getServerURL() { return serverURL; };
	static const bool getSound() { return sound; };
	void init();
	void start();
};

