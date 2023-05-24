#include <protocols/Wayland/GSeat.h>
#include <protocols/Wayland/RDataDevice.h>
#include <protocols/Wayland/RDataSource.h>

#include <private/LDNDManagerPrivate.h>
#include <private/LDataOfferPrivate.h>
#include <private/LDataDevicePrivate.h>

#include <LSurface.h>
#include <LClient.h>
#include <LDataSource.h>
#include <LSeat.h>
#include <LPointer.h>

using namespace Louvre;

LDNDManager::LDNDManager(Params *params)
{
    m_imp = new LDNDManagerPrivate();
    imp()->seat = params->seat;
}

LDNDManager::~LDNDManager()
{
    delete m_imp;
}

LDNDIconRole *LDNDManager::icon() const
{
    return imp()->icon;
}

LSurface *LDNDManager::origin() const
{
    return imp()->origin;
}

LSurface *LDNDManager::focus() const
{
    return imp()->focus;
}

LDataSource *LDNDManager::source() const
{
    return imp()->source;
}

LSeat *LDNDManager::seat() const
{
    return imp()->seat;
}

LClient *LDNDManager::dstClient() const
{
    return imp()->dstClient;
}

bool LDNDManager::dragging() const
{
    return imp()->origin != nullptr;
}

#if LOUVRE_DATA_DEVICE_MANAGER_VERSION >= 3

Louvre::LDNDManager::Action LDNDManager::preferredAction() const
{
    return imp()->preferredAction;
}

#endif

void LDNDManager::cancel()
{
    if (imp()->focus)
        imp()->focus->client()->dataDevice().imp()->sendDNDLeaveEvent();

    if (source())
        source()->dataSourceResource()->sendCancelled();

    imp()->clear();
    seat()->pointer()->setFocusC(nullptr);
    cancelled();
}

void LDNDManager::drop()
{
    if (dragging() && !imp()->dropped)
    {
        imp()->dropped = true;

        if (imp()->focus)
        {
            for (Protocols::Wayland::GSeat *s : imp()->focus->client()->seatGlobals())
                if (s->dataDeviceResource())
                    s->dataDeviceResource()->sendDrop();

            if (source())
                source()->dataSourceResource()->sendDNDDropPerformed();
        }
        else
        {
            cancel();
        }
    }
}

#if LOUVRE_DATA_DEVICE_MANAGER_VERSION >= 3
void LDNDManager::setPreferredAction(Louvre::LDNDManager::Action action)
{
    imp()->preferredAction = action;

    if (imp()->dstClient)
    {
        for (Protocols::Wayland::GSeat *s : dstClient()->seatGlobals())
        {
            if (s->dataDeviceResource() && s->dataDeviceResource()->dataOffered())
                s->dataDeviceResource()->dataOffered()->imp()->updateDNDAction();
        }
    }
}
#endif

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

LDNDManager::LDNDManagerPrivate *LDNDManager::imp() const
{
    return m_imp;
}
