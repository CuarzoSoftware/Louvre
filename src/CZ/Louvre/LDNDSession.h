#ifndef LDNDSESSION_H
#define LDNDSESSION_H

/// @cond OMIT

#include <LDND.h>
#include <LSeat.h>
#include <LSurface.h>
#include <LDNDIconRole.h>
#include <CZ/Louvre/Protocols/Wayland/RDataOffer.h>
#include <CZ/Louvre/Protocols/Wayland/RDataSource.h>
#include <CZ/Louvre/Protocols/Wayland/RDataDevice.h>

using namespace Louvre::Protocols::Wayland;

class Louvre::LDNDSession final : public LObject
{
public:
    LDNDSession() noexcept
    {
        source.setOnDestroyCallback([this](auto) { cancel(); });
        origin.setOnDestroyCallback([this](auto) { cancel(); });
        srcDataDevice.setOnDestroyCallback([this](auto) { cancel(); });
        action = LDND::Move;
    }

    void updateActions() noexcept
    {
        if (!source || !offer)
            return;

        offer->sourceActions(source->actions());
        action = 0;

        if (source->version() >= 3 && offer->version() >= 3)
            action = source->actions() & offer->actions();
        else if (source->version() >= 3)
            action = source->actions();
        else if (offer->version() >= 3)
            action = offer->actions();

        if (!compositorAction && (offer->preferredAction() & action))
            action &= offer->preferredAction();

        if (compositorAction & action)
            action &= compositorAction;

        if (action & LDND::Copy)
            action = LDND::Copy;
        else if (action & LDND::Move)
            action = LDND::Move;
        else if (action & LDND::Ask)
            action = LDND::Ask;

        source->action(action);
        offer->action(action);
    }

    inline void cancel() noexcept
    {
        if (seat()->dnd()->m_session.get() == this)
            seat()->dnd()->cancel();
    }

    UInt32 compositorAction { seat()->dnd()->preferredAction() };
    UInt32 action { 0 };
    bool dropped { false };
    CZWeak<LSurface> focus;
    CZWeak<LSurface> origin;
    CZWeak<LDNDIconRole> icon;
    CZWeak<RDataDevice> srcDataDevice;
    CZWeak<RDataDevice> dstDataDevice;
    CZWeak<RDataSource> source;
    CZWeak<RDataOffer> offer;
};

/// @endcond OMIT

#endif // LDNDSESSION_H
