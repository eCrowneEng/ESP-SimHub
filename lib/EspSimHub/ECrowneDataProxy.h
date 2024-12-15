#include <FullLoopbackStream.h>

#if CONNECTION_TYPE == WIFI
#include <TcpSerialBridge2.h>
TcpSerialBridge2 instance(BRIDGE_PORT);
#else
#include <ESPNowSerialBridge.h>
uint8_t peerMac[] = ESPNOW_PEER_MAC;
ESPNowSerialBridge instance(peerMac, ESPNOW_WIFI_CHANNEL);
#endif

class ECrowneDataProxy {
    public:
        static void setup(FullLoopbackStream *outgoingStream, FullLoopbackStream *incomingStream) {
            instance.setup(outgoingStream, incomingStream);
        }

        static void loop() {
            instance.loop();
        }

        static void flush() {
            instance.flush();
        }

        static void begin(unsigned long baud) {
            instance.begin(baud);
        }
};

// these will override the Serial interface that SimHub uses to use our Streams
#define FlowSerialBegin [](unsigned long baud) { ECrowneDataProxy::begin(baud); }
#define StreamRead incomingStream.read
#define StreamAvailable incomingStream.available
#define FlowSerialFlush ECrowneDataProxy::flush
#define StreamFlush ECrowneDataProxy::flush
#define StreamWrite outgoingStream.write
#define StreamPrint outgoingStream.print