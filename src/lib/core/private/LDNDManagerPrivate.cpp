#include <private/LDNDManagerPrivate.h>

void LDNDManager::LDNDManagerPrivate::clear()
{
    focus = nullptr;
    source = nullptr;
    origin = nullptr;
    icon = nullptr;
    dstClient = nullptr;
    dropped = false;
    matchedMimeType = false;
    destDidNotRequestReceive = 0;
}
