#ifndef LPOPUPROLEPRIVATE_H
#define LPOPUPROLEPRIVATE_H

#include <protocols/XdgShell/RXdgSurface.h>
#include <protocols/XdgShell/RXdgPopup.h>
#include <LPositioner.h>
#include <LPopupRole.h>
#include <list>

using namespace Louvre;

struct LPopupRole::Params
{
    LResource *popup;
    LSurface *surface;
    LPositioner *positioner;
};

LPRIVATE_CLASS(LPopupRole)

    enum StateFlags : UInt8
    {
        Dismissed               = static_cast<UInt8>(1) << 0,
        HasPendingReposition    = static_cast<UInt8>(1) << 1,
        HasConfigurationToSend  = static_cast<UInt8>(1) << 2,
        CanBeConfigured         = static_cast<UInt8>(1) << 3
    };

    LPopupRole *popup;
    Configuration current, pending, previous;
    std::list<Configuration> sentConfs;
    LRect positionerBounds;
    LPositioner positioner;
    UInt32 repositionToken;
    LBitset<StateFlags> stateFlags;

    void sendConfiguration() noexcept
    {
        if (!stateFlags.check(HasConfigurationToSend))
            return;

        stateFlags.remove(HasConfigurationToSend);

        if (stateFlags.check(HasPendingReposition))
        {
            stateFlags.remove(HasPendingReposition);
            popup->xdgPopupResource()->repositioned(repositionToken);
        }

        popup->xdgPopupResource()->configure(pending.rect);
        popup->xdgSurfaceResource()->configure(pending.serial);
        sentConfs.emplace_back(pending);
    }
};

#endif // LPOPUPROLEPRIVATE_H
