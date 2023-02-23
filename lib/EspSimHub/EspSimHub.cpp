#include <EspSimHub.h>

String getUniqueId()
{
    return WiFi.macAddress();
}