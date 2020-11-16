#include <rfgate.h>
#include <app.h>

Timer receiveTimer;
Timer beeperTimer;

RCSwitch rfTransceiver;
const int beeperPin{4};
const int receivePin{12};

void sendRF()
{
	rfTransceiver.send(5393, 24);
	Serial.println("RF command successful sent");
}

void beep()
{
	digitalWrite(beeperPin,LOW);
}

void httpPost(unsigned long value)
{
	Serial.print("Server URL - ");
	Serial.print(AppClass::getServerURL());
	Serial.print("\n");
	HttpClient httpClient;
	//HttpRequest* postRequest = new HttpRequest(F("http://10.2.113.100:3000/"));
	HttpRequest* postRequest = new HttpRequest(AppClass::getServerURL());
	postRequest
			->setMethod(HTTP_POST)
			->onRequestComplete(nullptr)
			->setPostParameter("id", String{value});
	httpClient.send(postRequest);
}

void receiveRF()
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
			digitalWrite(beeperPin,HIGH);
			beeperTimer.initializeMs(100, beep).start();
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
}

void AppClass::_saveAppConfig(file_t& file)
{
	size_t strSize{serverURL.length()};
	fileWrite(file, &strSize, sizeof(strSize));
	fileWrite(file, serverURL.c_str(), strSize);
}

bool AppClass::_extraConfigReadJson(JsonObject& json)
{
	bool needSave{false};
	json.prettyPrintTo(Serial); Serial.println();
	if (json["serverURL"].success())
	{
		serverURL = String((const char *)json["serverURL"]);
		needSave = true;
	}

	return needSave;
}

void AppClass::_extraConfigWriteJson(JsonObject& json)
{
	json["serverURL"] = serverURL;
}

void AppClass::init()
{
	ApplicationClass::init();

	rfTransceiver.enableReceive(receivePin);

	pinMode(beeperPin,OUTPUT);
	digitalWrite(beeperPin,LOW);

	// Optional set pulse length.
	//mySwitch.setPulseLength(240);
	// Optional set protocol (default is 1, will work for most outlets)
	// mySwitch.setProtocol(2);

//	sendTimer.initializeMs(1000, sendRF).start();
	receiveTimer.initializeMs(20, receiveRF).start();

	Serial.printf(_F("AppClass init done!\n"));
}

void AppClass::start()
{
	ApplicationClass::start();
}
