#pragma once
#include <application.h>

class AppClass : public ApplicationClass
{
	static String serverURL;
	void _loadAppConfig(file_t& file) override;
	void _saveAppConfig(file_t& file) override;
	bool _extraConfigReadJson(JsonObject& json) override;
	void _extraConfigWriteJson(JsonObject& json) override;
public:
	static const String getServerURL() { return serverURL; };
	void init();
	void start();
};

