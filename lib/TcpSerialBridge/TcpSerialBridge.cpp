#include <EspSimHub.h>
#include <BoardWifi.h>
#include <WiFiManager.h>
#include <TcpSerialBridge.h>

void assertConnected(boolean connected)
{
	if (!connected)
	{
		Serial.println("Not connected after portal closed, restarting");
		ESP.restart();
		return;
	}
}

TcpSerialBridge::TcpSerialBridge(
	uint16_t tcpPort,
	Stream *outgoingStream,
	Stream *incomingStream,
	bool debug) : server(tcpPort)
{
	WiFiManager wifiManager;
	ssid = "SHC-" + getUniqueId();
	this->outgoingStream = outgoingStream;
	this->incomingStream = incomingStream;
	this->debug = debug;
}

void TcpSerialBridge::setup(bool resetWiFiSettings)
{
	if (debug)
	{
		wifiManager.setDebugOutput(true);
	}
	bool connected;
	WiFi.hostname(ssid);

	// Delete existing credentials
	if (resetWiFiSettings)
	{
		wifiManager.resetSettings();
	}

	connected = wifiManager.autoConnect(ssid.c_str(), NULL);
	assertConnected(connected);

	if (debug)
	{
		Serial.println("Wifi is known and connected");
	}
	server.begin();
	server.setNoDelay(true);
};

void TcpSerialBridge::loop(bool startConfigPortalAgain)
{
	// This can be used to trigger the wifi config portal again during runtime
	//  it will block the loop until wifi is connected again
	// Useful if you can read a pin (button) and trigger this if you need
	if (startConfigPortalAgain)
	{
		bool connected;
		wifiManager.startConfigPortal(ssid.c_str(), NULL);
	}

	yield();

	// Check if there are any new clients
	if (server.hasClient())
	{
		client = server.available();
		if (client && !client.connected())
		{
			if (debug)
			{
				Serial.println("client disconnected");
			}
			client.stop();
		}
	}
	else
	{
		// no free/disconnected spot so reject
		WiFiClient rejectClient = server.available();
		if (rejectClient)
		{
			rejectClient.stop();
			if (debug)
			{
				Serial.println("client rejected");
			}
		}
	}

	bool clientConnected = (client && client.connected());
	if (clientConnected)
	{
		if (!firstConnectionDone)
		{
			firstConnectionDone = true;
			onFirstConnection();
		}
		if (client.available())
		{
			// if client data is available in tcp server
			//  it's intended for the Serial stream
			while (client.available())
			{
				// pipe data from tcp to serial stream
				int ch = client.read();
				this->incomingStream->write(ch);
			}
		}
	}

	// if there is data available in the wifi stream, it's meant
	//  to go from Serial port to TCP client
	size_t availableLength = this->outgoingStream->available();
	if (availableLength)
	{
		// read the available data from the stream, and put in in the buffer
		uint8_t sbuf[availableLength];
		this->outgoingStream->readBytes(sbuf, availableLength);

		// if TCP client is available
		if (client && client.connected())
		{
			// write all data to client in one go
			client.write(sbuf, availableLength);
		}
	}
}

void TcpSerialBridge::onFirstConnection()
{
	if (debug)
	{
		Serial.println("Client connected");
	}
}