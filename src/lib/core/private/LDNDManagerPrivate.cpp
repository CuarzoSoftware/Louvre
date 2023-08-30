#include <private/LDNDManagerPrivate.h>

void LDNDManager::LDNDManagerPrivate::clear()
{
    focus = nullptr;
    source = nullptr;
    origin = nullptr;
    icon = nullptr;
    srcDataDevice = nullptr;
    dstClient = nullptr;
    matchedMimeType = false;
}
