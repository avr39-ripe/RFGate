#include <SmingCore.h>
#include <Libraries/RCSwitch/RCSwitch.h>

Timer sendTimer;
Timer receiveTimer;
Timer beeperTimer;

RCSwitch mySwitch = RCSwitch();
const int beeperPin{4};

void sendRF()
{
	mySwitch.send(5393, 24);
	Serial.println("RF command successful sent");
}

void beep()
{
	digitalWrite(beeperPin,LOW);
}

void httpPost(unsigned long value)
{
	HttpClient httpClient;
	HttpRequest* postRequest = new HttpRequest(F("http://10.2.113.100:3000/"));
	postRequest
			->setMethod(HTTP_POST)
			->onRequestComplete(nullptr)
			->setPostParameter("id", String{value});
	httpClient.send(postRequest);
}

void receiveRF()
{
	if(mySwitch.available()) {
		if(mySwitch.getReceivedValue() == 0) {
			Serial.print("Unknown encoding");
		} else {
			Serial.print("Received ");
			Serial.print(mySwitch.getReceivedValue());
			Serial.print(" / ");
			Serial.print(mySwitch.getReceivedBitlength());
			Serial.print("bit ");
			Serial.print("Protocol: ");
			Serial.println(mySwitch.getReceivedProtocol());
			digitalWrite(beeperPin,HIGH);
			beeperTimer.initializeMs(100, beep).start();
			httpPost(mySwitch.getReceivedValue());
		}

		mySwitch.resetAvailable();
	}
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Enable debug output to serial

//	mySwitch.enableTransmit(5); // pin GPIO5 - transmit
	mySwitch.enableReceive(12);  // pin GPIO4  - receive

	pinMode(beeperPin,OUTPUT);
	digitalWrite(beeperPin,LOW);
	
	// Optional set pulse length.
	//mySwitch.setPulseLength(240);
	// Optional set protocol (default is 1, will work for most outlets)
	// mySwitch.setProtocol(2);

//	sendTimer.initializeMs(1000, sendRF).start();
	receiveTimer.initializeMs(20, receiveRF).start();

        WifiStation.config(WIFI_SSID, WIFI_PWD);
        WifiStation.enable(true, true);
}
