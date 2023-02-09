#include <BoardWifi.h>
#include <WiFiManager.h>

// these will override the Serial interface that SimHub uses to use our Streams
#define FlowSerialBegin [](unsigned long baud) {}
#define StreamRead incomingStream.read
#define StreamAvailable incomingStream.available
#define FlowSerialFlush outgoingStream.flush
#define StreamFlush outgoingStream.flush
#define StreamWrite outgoingStream.write
#define StreamPrint outgoingStream.print

class TcpSerialBridge
{
public:
  TcpSerialBridge(
      uint16_t tcpPort,
      Stream *outgoingStream,
      Stream *incomingStream,
      bool debug);
  void setup(bool resetWiFiSettings);
  void loop(bool startConfigPortalAgain);

private:
  WiFiManager wifiManager;
  WiFiServer server;
  WiFiClient client;
  String ssid;
  Stream *outgoingStream;
  Stream *incomingStream;
  bool debug;
};
