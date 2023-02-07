#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <TcpSerialBridge.h>

#define MAX_PENDING_CLIENTS_PER_PORT 1

#ifndef LED_PIN
// Used for status, use -1 to disable
#define LED_PIN -1
#endif

void assertConnected(boolean connected) {
	if (!connected) {
		Serial.println("Not connected after portal closed, restarting");
		ESP.restart();
		return;
	}
}

TcpSerialBridge::TcpSerialBridge(
	uint16_t tcpPort,
	Stream *outgoingStream,
	Stream *incomingStream
) : server(tcpPort) {
	WiFiManager wifiManager;
	WiFiClient serverClient();
	ssid = "SHC-" + WiFi.macAddress();
	this->outgoingStream = outgoingStream;
	this->incomingStream = incomingStream;
}

void TcpSerialBridge::setup(bool resetWiFiSettings) {
#if DEBUG_TCP_BRIDGE
	wifiManager.setDebugOutput(true);
#endif
	bool connected;	
	WiFi.hostname(ssid);

	// Delete existing credentials 
	if (resetWiFiSettings) {
		wifiManager.resetSettings();
	}

  	connected = wifiManager.autoConnect(ssid.c_str(), NULL);	
	assertConnected(connected);

#if DEBUG_TCP_BRIDGE
    Serial.println("Wifi is known and connected");
#endif

	server.begin();
	server.setNoDelay(true);

	pinMode(LED_PIN, OUTPUT);
	digitalWrite(LED_PIN, LOW);
};

void TcpSerialBridge::loop(bool startConfigPortalAgain) {
	// This can be used to trigger the wifi config portal again during runtime
	//  it will block the loop until wifi is connected again
	// Useful if you can read a pin (button) and trigger this if you need
	if (startConfigPortalAgain) {
		bool connected;
		connected = wifiManager.startConfigPortal(ssid.c_str());
		assertConnected(connected);
	}

	// Check if there are any new clients
	if (server.hasClient()) {
		if (client && !client.connected()) {
#if DEBUG_TCP_BRIDGE
		Serial.println("client disconnected");
#endif
			client.stop();
		}
#if DEBUG_TCP_BRIDGE
		Serial.println("client connected");
#endif
	} else {
		//no free/disconnected spot so reject
		WiFiClient rejectClient = server.available();
		if (rejectClient) {
			rejectClient.stop();
#if DEBUG_TCP_BRIDGE
		Serial.println("client rejected");
#endif
		}
	}

	// Turn off led if there is a connection
	bool clientConnected = (client && client.connected());
	digitalWrite(LED_PIN, clientConnected);

	if (clientConnected) {
		if (client.available()) {
			// if client data is available in tcp server
			//  it's intended for the Serial stream
			while (client.available()) {
				// pipe data from tcp to serial stream
				int ch = client.read();
				this->incomingStream->write(ch);
			}
		}
	}

	// if there is data available in the wifi stream, it's meant
	//  to go from Serial port to TCP client
	size_t availableLength = this->outgoingStream->available();
	if (availableLength) {
		// read the available data from the stream, and put in in the buffer
		uint8_t sbuf[availableLength];
		this->outgoingStream->readBytes(sbuf, availableLength);

		// if TCP client is available
		if (client && client.connected()) {			
			// write all data to client in one go
			client.write(sbuf, availableLength);
			//  just to be safe
			delay(1);
		}
	}
}
