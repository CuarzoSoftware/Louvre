#ifndef LDNDSESSION_H
#define LDNDSESSION_H

#include <LDND.h>
#include <LSeat.h>
#include <LSurface.h>
#include <LDNDIconRole.h>
#include <protocols/Wayland/RDataOffer.h>
#include <protocols/Wayland/RDataSource.h>
#include <protocols/Wayland/RDataDevice.h>

using namespace Louvre::Protocols::Wayland;

class Louvre::LDNDSession : public LObject
{
public:
    inline LDNDSession() noexcept
    {
        source.setOnDestroyCallback([this](auto) { cancel(); });
        origin.setOnDestroyCallback([this](auto) { cancel(); });
        srcDataDevice.setOnDestroyCallback([this](auto) { cancel(); });
    }

    void updateActions() noexcept
    {
        if (!source.get() || !offer.get())
            return;

        offer.get()->sourceActions(source.get()->actions());
        action = 0;

        if (source.get()->version() >= 3 && offer.get()->version() >= 3)
            action = source.get()->actions() & offer.get()->actions();
        else if (source.get()->version() >= 3)
            action = source.get()->actions();
        else if (offer.get()->version() >= 3)
            action = offer.get()->actions();

        if (!compositorAction && (offer.get()->preferredAction() & action))
            action &= offer.get()->preferredAction();

        if (compositorAction & action)
            action &= compositorAction;

        if (action & LDND::Copy)
            action = LDND::Copy;
        else if (action & LDND::Move)
            action = LDND::Move;
        else if (action & LDND::Ask)
            action = LDND::Ask;

        source.get()->action(action);
        offer.get()->action(action);
    }

    inline void cancel() noexcept
    {
        if (seat()->dnd()->m_session.get() == this)
            seat()->dnd()->cancel();
    }

    UInt32 compositorAction { seat()->dnd()->preferredAction() };
    UInt32 action { 0 };
    bool dropped { false };
    LWeak<LSurface> focus;
    LWeak<LSurface> origin;
    LWeak<LDNDIconRole> icon;
    LWeak<RDataDevice> srcDataDevice;
    LWeak<RDataDevice> dstDataDevice;
    LWeak<RDataSource> source;
    LWeak<RDataOffer> offer;
};

#endif // LDNDSESSION_H
