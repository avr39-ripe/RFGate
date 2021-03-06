#include <rfgate.h>
#include <app.h>

String AppClass::serverURL{"http://10.2.113.100:3000"};
bool AppClass::sound{true};
uint32_t AppClass::wsCheckConnectionInterval{2000};
uint32_t AppClass::wsBroadcastPingInterval{1000};
bool AppClass::wsBinaryFormat{false};

Timer receiveTimer;
Timer beeperTimer;

RCSwitch rfTransceiver;


void AppClass::httpPost(unsigned long value)
{
	HttpClient httpClient;
	HttpRequest* postRequest = new HttpRequest(AppClass::serverURL);
	postRequest
			->setMethod(HTTP_POST)
			->onRequestComplete(nullptr)
			->setPostParameter("id", String{value});
	httpClient.send(postRequest);
}

void AppClass::receiveRF()
{
	uint32_t receivedValue;

	if(rfTransceiver.available()) {
		if( (receivedValue = rfTransceiver.getReceivedValue()) == 0) {
			Serial.print("Unknown encoding");
		} else {
			Serial.print("Received ");
			Serial.print(receivedValue);
			Serial.print(" / ");
			Serial.print(rfTransceiver.getReceivedBitlength());
			Serial.print("bit ");
			Serial.print("Protocol: ");
			Serial.println(rfTransceiver.getReceivedProtocol());

			if (AppClass::sound)
			{
				digitalWrite(AppClass::beeperPin,HIGH);
				//beeperTimer.initializeMs(100, beep).start();
				beeperTimer.initializeMs(100, [=](){digitalWrite(AppClass::beeperPin,LOW);}).start(false);
			}

			//httpPost(rfTransceiver.getReceivedValue());
			if (wsBinaryFormat)
			{
				WebsocketConnection::broadcast(reinterpret_cast<const char*>(&receivedValue), sizeof(receivedValue), WS_FRAME_BINARY);
			}
			else
			{
				WebsocketConnection::broadcast(String{receivedValue});
			}



		}

		rfTransceiver.resetAvailable();
	}
}

void AppClass::_loadAppConfig(file_t& file)
{
	size_t strSize;

	fileRead(file, &strSize, sizeof(strSize));
	uint8_t* serverURLBuffer = new uint8_t[strSize+1];
	fileRead(file, serverURLBuffer, strSize);
	serverURLBuffer[strSize] = 0;
	serverURL = (const char *)serverURLBuffer;
	delete[] serverURLBuffer;
	fileRead(file, &sound, sizeof(sound));
	fileRead(file, &wsBinaryFormat, sizeof(wsBinaryFormat));
	fileRead(file, &wsBroadcastPingInterval, sizeof(wsBroadcastPingInterval));
	fileRead(file, &wsCheckConnectionInterval, sizeof(wsCheckConnectionInterval));

}

void AppClass::_saveAppConfig(file_t& file)
{
	size_t strSize{serverURL.length()};
	fileWrite(file, &strSize, sizeof(strSize));
	fileWrite(file, serverURL.c_str(), strSize);
	fileWrite(file, &sound, sizeof(sound));
	fileWrite(file, &wsBinaryFormat, sizeof(wsBinaryFormat));
	fileWrite(file, &wsBroadcastPingInterval, sizeof(wsBroadcastPingInterval));
	fileWrite(file, &wsCheckConnectionInterval, sizeof(wsCheckConnectionInterval));
}

bool AppClass::_extraConfigReadJson(JsonObject& json)
{
	bool needSave{false};
	json.prettyPrintTo(Serial); Serial.println();
	if (json["serverURL"].success())
	{
		serverURL = static_cast<const char *>(json["serverURL"]);
		needSave = true;
	}

	if (json["sound"].success())
	{
		sound = static_cast<bool>(json["sound"]);
		needSave = true;
	}

	if (json["wsBinaryFormat"].success())
	{
		wsBinaryFormat = static_cast<bool>(json["wsBinaryFormat"]);
		needSave = true;
	}

	if (json["wsBroadcastPingInterval"].success())
	{
		wsBroadcastPingInterval = static_cast<uint32_t>(json["wsBroadcastPingInterval"]);
		if (wsBroadcastPingInterval)
		{
			_wsBroadcastPingTimer.stop();
			_wsBroadcastPingTimer.initializeMs(wsBroadcastPingInterval, wsBroadcastPing).start();
		}
		needSave = true;
	}

	if (json["wsCheckConnectionInterval"].success())
	{
		wsCheckConnectionInterval = static_cast<uint32_t>(json["wsCheckConnectionInterval"]);
		if (wsBroadcastPingInterval)
		{
			_wsCheckConnectionTimer.stop();
			_wsCheckConnectionTimer.initializeMs(wsCheckConnectionInterval, wsCheckConnection).start();
		}
		needSave = true;
	}
	return needSave;
}

void AppClass::_extraConfigWriteJson(JsonObject& json)
{
	json["serverURL"] = serverURL;
	json["sound"] = sound;
	json["wsBinaryFormat"] = wsBinaryFormat;
	json["wsBroadcastPingInterval"] = wsBroadcastPingInterval;
	json["wsCheckConnectionInterval"] = wsCheckConnectionInterval;
}

void AppClass::wsConnected(WebsocketConnection& connection)
{
	Serial.print(_F("WebSocket Connected!\n"));
	Serial.print(_F("WebSocket userData created!\n"));
	connection.setUserData(new uint8_t{5});
}

void AppClass::wsDisconnected(WebsocketConnection& connection)
{
	Serial.print(_F("WebSocket Disconnected!\n"));
	auto wsState{reinterpret_cast<uint8_t*>(connection.getUserData())};
	if(wsState)
	{
		Serial.print(_F("WebSocket userData deleted!\n"));
		delete wsState;
	}

}

void AppClass::wsPong(WebsocketConnection& connection)
{
	Serial.print(_F("Got websocket pong reply\n"));
	auto wsState{reinterpret_cast<uint8_t*>(connection.getUserData())};
	Serial.print(_F("wsState was: "));
	Serial.println(static_cast<int>(*wsState));
	if (*wsState < 5 )
	{
		++(*wsState);
	}
}
void AppClass::wsCheckConnection()
{
	auto websocketList{WebsocketConnection::getActiveWebsockets()};

	for(unsigned i{0}; i < websocketList.count(); ++i)
	{
		auto wsState{reinterpret_cast<bool*>(websocketList[i]->getUserData())};
		if (!(*wsState))
		{
			Serial.print(_F("Dead WebSocket connection detected! Disconnecting!\n"));
			websocketList[i]->close();
		}

	}
}

void AppClass::init()
{
	ApplicationClass::init();

	rfTransceiver.enableReceive(receivePin);

	pinMode(beeperPin,OUTPUT);
	digitalWrite(beeperPin,LOW);

	receiveTimer.initializeMs(receiveRefresh, receiveRF).start();

	_wsResource->setConnectionHandler(WebsocketDelegate(&AppClass::wsConnected,this));
	_wsResource->setDisconnectionHandler(WebsocketDelegate(&AppClass::wsDisconnected,this));
	_wsResource->setPongHandler(WebsocketDelegate(&AppClass::wsPong,this));

	if (wsBroadcastPingInterval)
	{
		_wsBroadcastPingTimer.initializeMs(wsBroadcastPingInterval, wsBroadcastPing).start();
	}
	if (wsCheckConnectionInterval)
	{
		_wsCheckConnectionTimer.initializeMs(wsCheckConnectionInterval, wsCheckConnection).start();
	}

	Serial.printf(_F("AppClass init done!\n"));
}

void AppClass::start()
{
	ApplicationClass::start();
}

void AppClass::wsBroadcastPing()
{
	const uint8_t pingData{42};
	Serial.print("Broadcast websocket ping for -> ");

	auto websocketList{WebsocketConnection::getActiveWebsockets()};

	Serial.println(websocketList.count());

	for(unsigned i{0}; i < websocketList.count(); ++i)
	{
		websocketList[i]->send(reinterpret_cast<const char*>(&pingData), sizeof(uint8_t), WS_FRAME_PING);
		auto wsState{reinterpret_cast<uint8_t*>(websocketList[i]->getUserData())};
		Serial.print(_F("wsState was: "));
		Serial.println(static_cast<int>(*wsState));
		if ( *wsState )
		{
			--(*wsState);
		}
	}
}
