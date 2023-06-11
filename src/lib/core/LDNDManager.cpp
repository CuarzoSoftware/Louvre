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
    L_UNUSED(params);
    m_imp = new LDNDManagerPrivate();
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

LClient *LDNDManager::dstClient() const
{
    return imp()->dstClient;
}

bool LDNDManager::dragging() const
{
    return imp()->origin != nullptr;
}

void LDNDManager::cancel()
{
    if (imp()->focus)
        imp()->focus->client()->dataDevice().imp()->sendDNDLeaveEvent();

    if (source())
        source()->dataSourceResource()->cancelled();

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
            for (Wayland::GSeat *s : imp()->focus->client()->seatGlobals())
                if (s->dataDeviceResource())
                    s->dataDeviceResource()->drop();

            if (source())
                source()->dataSourceResource()->dndDropPerformed();
        }
        else
        {
            cancel();
        }
    }
}

// Since 3

LDNDManager::Action LDNDManager::preferredAction() const
{
    return imp()->preferredAction;
}

void LDNDManager::setPreferredAction(LDNDManager::Action action)
{
    imp()->preferredAction = action;

    if (imp()->dstClient)
    {
        for (Wayland::GSeat *s : dstClient()->seatGlobals())
        {
            if (s->dataDeviceResource() && s->dataDeviceResource()->dataOffered())
                s->dataDeviceResource()->dataOffered()->imp()->updateDNDAction();
        }
    }
}
