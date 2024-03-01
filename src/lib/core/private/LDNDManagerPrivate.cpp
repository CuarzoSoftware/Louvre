#include <private/LDNDManagerPrivate.h>
#include <protocols/Wayland/GSeat.h>
#include <protocols/Wayland/RDataDevice.h>
#include <LClient.h>
#include <LSurface.h>

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

void LDNDManager::LDNDManagerPrivate::sendLeaveEvent(LSurface *surface)
{
    matchedMimeType = false;
    focus = nullptr;

    if (!surface)
        return;

    for (auto seatGlobal : surface->client()->seatGlobals())
        if (seatGlobal->dataDeviceResource())
            seatGlobal->dataDeviceResource()->leave();
}
