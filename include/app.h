#pragma once
#include <application.h>
#include "UniqueArray.h"

class AppClass : public ApplicationClass
{
	static const int beeperPin{4};
	static const int receivePin{12};
	static const int receiveRefresh{20}; // Try to receive every receiveRefresh ms.
	static const uint8_t recvQueueSize{50};
	static UniqueArray<uint32_t,recvQueueSize> recvQueue;

	static String serverURL;
	static bool sound;
	static uint32_t sendDataInterval;

	static void OnCompleted(TcpClient& client, bool successful);
	static void OnReadyToSend(TcpClient& client, TcpConnectionEvent sourceEvent);
	static bool OnReceive(TcpClient& client, char* buf, int size);

	static void tcpSend();
	static TcpClient tcpClient;

	void _loadAppConfig(file_t& file) override;
	void _saveAppConfig(file_t& file) override;
	bool _extraConfigReadJson(JsonObject& json) override;
	void _extraConfigWriteJson(JsonObject& json) override;
	static void receiveRF();

public:
	static const String getServerURL() { return serverURL; };
	static const bool getSound() { return sound; };
	void init();
	void start();
};

