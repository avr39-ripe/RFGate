#include <rfgate.h>
#include <app.h>

String AppClass::serverURL{"data://192.168.0.106:3000"};
bool AppClass::sound{true};
uint32_t AppClass::sendDataInterval{10000};
UniqueArray<uint32_t,AppClass::recvQueueSize> AppClass::recvQueue;

Timer receiveTimer;
Timer beeperTimer;

RCSwitch rfTransceiver;

// TCP CLIENT

Timer tcpSendTimer;

void AppClass::OnCompleted(TcpClient& client, bool successful)
{
	// debug msg
	debug_e("OnCompleted");
	debug_e("successful: %d", successful);

	if (!successful)
	{
		Serial.print("TIMEOUT TCPClient!\n");
		client.close();
		AppClass::recvQueue.flush();
	}
}

void AppClass::OnReadyToSend(TcpClient& client, TcpConnectionEvent sourceEvent)
{
	// debug msg
	debug_e("OnReadyToSend");
	debug_e("sourceEvent: %d", sourceEvent);

	if(sourceEvent == eTCE_Connected)
	{
		uint32_t id;
		String sendId;
		Serial.print("RECVQueue.count: ");
		Serial.println(AppClass::recvQueue.count());

		for(size_t i{0}; i<AppClass::recvQueue.count(); ++i)
		{
			id = AppClass::recvQueue[i];
			Serial.print("Sending: ");
			Serial.println(id);
			sendId += id;
			sendId += '\n';
		}
		AppClass::recvQueue.flush();
		Serial.print("client.sendString()\n");
		client.sendString(sendId,true);
	}
}

bool AppClass::OnReceive(TcpClient& client, char* buf, int size)
{
	// debug msg
	debug_e("OnReceive");
	debug_e("%s", buf);
	return true;
}

TcpClient AppClass::tcpClient(AppClass::OnCompleted, AppClass::OnReadyToSend, AppClass::OnReceive);

void AppClass::tcpSend()
{
	Serial.printf("Free Heap: %d\n", system_get_free_heap_size());
	if(AppClass::recvQueue.count())
	{
		Serial.print("recvQueue not empty! Try to send data\n");
		if (AppClass::tcpClient.isProcessing())
		{
			Serial.print("TCPClient already active! Give'em a chance!\n");
		}
		else
		{
			Serial.print("SENDING via TCPClient!\n");
			Url tcpUrl{AppClass::getServerURL()};
			AppClass::tcpClient.connect(tcpUrl.Host, tcpUrl.Port);
		}

	}
	else
	{
		Serial.print("recvQueue EMPTY!\n");
	}


}
// TCP CLIENT

void AppClass::receiveRF()
{
	uint32_t receivedValue;

	if(rfTransceiver.available())
	{
		if( (receivedValue = rfTransceiver.getReceivedValue()) == 0)
		{
			Serial.print("Unknown encoding");
		}
		else
		{
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
				beeperTimer.initializeMs(100, [=](){digitalWrite(AppClass::beeperPin,LOW);}).start(false);
			}

			if (recvQueue.full())
			{
				Serial.print("Queue full!\n");
			}
			else
			{
				if (recvQueue.add(receivedValue) )
				{
					Serial.print("Add new code\n");
				}
				else
				{
					Serial.print("Repeated code, skip for now!\n");
				}
			}
			rfTransceiver.resetAvailable();
		}
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
	fileRead(file, &sendDataInterval, sizeof(sendDataInterval));
}

void AppClass::_saveAppConfig(file_t& file)
{
	size_t strSize{serverURL.length()};
	fileWrite(file, &strSize, sizeof(strSize));
	fileWrite(file, serverURL.c_str(), strSize);
	fileWrite(file, &sound, sizeof(sound));
	fileWrite(file, &sendDataInterval, sizeof(sendDataInterval));
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


	if (json["sendDataInterval"].success())
	{
		sendDataInterval = static_cast<uint32_t>(json["sendDataInterval"]);
		if (sendDataInterval)
		{
			tcpSendTimer.stop();
			tcpSendTimer.initializeMs(sendDataInterval, tcpSend).start();
		}
		needSave = true;
	}

	return needSave;
}

void AppClass::_extraConfigWriteJson(JsonObject& json)
{
	json["serverURL"] = serverURL;
	json["sound"] = sound;
	json["sendDataInterval"] = sendDataInterval;
}


void AppClass::init()
{
	ApplicationClass::init();

	rfTransceiver.enableReceive(receivePin);

	pinMode(beeperPin,OUTPUT);
	digitalWrite(beeperPin,LOW);

	receiveTimer.initializeMs(receiveRefresh, receiveRF).start();

	tcpSendTimer.initializeMs(sendDataInterval, tcpSend).start();

	Serial.printf(_F("AppClass init done!\n"));
}

void AppClass::start()
{
	ApplicationClass::start();
}

