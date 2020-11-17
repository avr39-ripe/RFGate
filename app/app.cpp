#include <rfgate.h>
#include <app.h>

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
	if(rfTransceiver.available()) {
		if(rfTransceiver.getReceivedValue() == 0) {
			Serial.print("Unknown encoding");
		} else {
			Serial.print("Received ");
			Serial.print(rfTransceiver.getReceivedValue());
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

			httpPost(rfTransceiver.getReceivedValue());
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
}

void AppClass::_saveAppConfig(file_t& file)
{
	size_t strSize{serverURL.length()};
	fileWrite(file, &strSize, sizeof(strSize));
	fileWrite(file, serverURL.c_str(), strSize);
	fileWrite(file, &sound, sizeof(sound));
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

	return needSave;
}

void AppClass::_extraConfigWriteJson(JsonObject& json)
{
	json["serverURL"] = serverURL;
	json["sound"] = sound;
}

void AppClass::init()
{
	ApplicationClass::init();

	rfTransceiver.enableReceive(AppClass::receivePin);

	pinMode(AppClass::beeperPin,OUTPUT);
	digitalWrite(AppClass::beeperPin,LOW);

	receiveTimer.initializeMs(AppClass::receiveRefresh, AppClass::receiveRF).start();

	Serial.printf(_F("AppClass init done!\n"));
}

void AppClass::start()
{
	ApplicationClass::start();
}
