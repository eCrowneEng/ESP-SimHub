#include "EspNowProtocol.h"

unsigned char* numberToBytes(int number) {
    static unsigned char bytes[3];
    bytes[0] = (number >> 16) & 0xFF;
    bytes[1] = (number >> 8) & 0xFF;
    bytes[2] = number & 0xFF;
    return bytes;
}

int bytesToNumber(unsigned char* bytes) {
    int number = 0;
    number |= bytes[0] << 16;
    number |= bytes[1] << 8; 
    number |= bytes[2];
    return number;
}


uint8_t* makeCommand(
    uint8_t* commandPayloadBytes, 
    int commandPayloadBytesCount, 
    uint8_t command, 
    size_t& resultingBytesCount
) {
    static uint8_t commandBytes[MAX_BRIDGE_BYTES];
    
    // Header
    commandBytes[0] = COMMAND_HEADER;
    
    // Length 
    commandBytes[1] = commandPayloadBytesCount;
    
    // Bridge command byte
    commandBytes[2] = command;
    
    // Copy data bytes
    memcpy(&commandBytes[3], commandPayloadBytes, commandPayloadBytesCount);
    
    // End marker
    commandBytes[3 + commandPayloadBytesCount] = COMMAND_END;
    
    resultingBytesCount = 3 + commandPayloadBytesCount + 1;
    return commandBytes;
}
