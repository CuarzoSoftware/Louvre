#include <protocols/Wayland/GSeat.h>
#include <protocols/Wayland/RDataDevice.h>
#include <protocols/Wayland/RDataSource.h>
#include <private/LDNDManagerPrivate.h>
#include <private/LDataOfferPrivate.h>
#include <private/LDataDevicePrivate.h>
#include <private/LSurfacePrivate.h>
#include <private/LCompositorPrivate.h>
#include <LDNDIconRole.h>
#include <LClient.h>
#include <LDataSource.h>
#include <LSeat.h>
#include <LPointer.h>
#include <LTimer.h>

using namespace Louvre;

LDNDManager::LDNDManager(void *params) : LPRIVATE_INIT_UNIQUE(LDNDManager)
{
    L_UNUSED(params);
}

LDNDManager::~LDNDManager() {}

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

Wayland::RDataDevice *LDNDManager::srcDataDevice() const
{
    return imp()->srcDataDevice;
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
    {
        source()->dataSourceResource()->dndFinished();
        source()->dataSourceResource()->cancelled();
    }

    imp()->clear();
    cancelled();
}

void LDNDManager::drop()
{
    if (dragging() && !imp()->dropped)
    {
        imp()->dropped = true;

        LTimer::oneShot(100, [this](LTimer *)
        {
            if (source() && imp()->dropped)
                cancel();
        });

        compositor()->imp()->unlockPoll();

        if (icon() && icon()->surface())
            icon()->surface()->imp()->setMapped(false);

        if (imp()->focus)
        {
            for (Wayland::GSeat *s : imp()->focus->client()->seatGlobals())
            {
                if (s->dataDeviceResource())
                {
                    if (!imp()->matchedMimeType && s->dataDeviceResource()->version() >= 3)
                    {
                        cancel();
                        return;
                    }

                    s->dataDeviceResource()->drop();
                }
            }

            if (source())
                source()->dataSourceResource()->dndDropPerformed();

            LSurface *focus = seat()->pointer()->focus();
            seat()->pointer()->setFocus(nullptr);
            seat()->pointer()->setFocus(focus);
        }
        else
        {
            if (source())
                source()->dataSourceResource()->dndDropPerformed();

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
    if (imp()->preferredAction == action)
        return;

    imp()->preferredAction = action;

    if (focus())
        for (Wayland::GSeat *s : focus()->client()->seatGlobals())
            if (s->dataDeviceResource() && s->dataDeviceResource()->dataOffered())
                s->dataDeviceResource()->dataOffered()->imp()->updateDNDAction();
}
