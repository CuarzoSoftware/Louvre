#include <CZ/Louvre/Manager/LActivationTokenManager.h>
#include <CZ/Louvre/Manager/LSessionLockManager.h>
#include <CZ/Louvre/Seat/LSeat.h>
#include <CZ/Louvre/Seat/LPointer.h>
#include <CZ/Louvre/Seat/LKeyboard.h>
#include <CZ/Louvre/Seat/LTouch.h>
#include <CZ/Louvre/Seat/LTouchPoint.h>
#include <algorithm>

using namespace CZ;

//! [createTokenRequest]
void LActivationTokenManager::createTokenRequest()
{
    // Deny requests not originated from a recent user event

    bool allowed { false };

    if (token()->triggeringEvent())
    {
        if (token()->triggeringEvent()->isPointerEvent())
            allowed = seat()->pointer()->focus() && seat()->pointer()->focus()->client() == token()->creator();
        else if (token()->triggeringEvent()->isKeyboardEvent())
            allowed = seat()->keyboard()->focus() && seat()->keyboard()->focus()->client() == token()->creator();
        else if (token()->triggeringEvent()->isTouchEvent())
        {
            const auto &touchPoints { seat()->touch()->touchPoints() };
            allowed = std::any_of(touchPoints.begin(), touchPoints.end(), [this](LTouchPoint *tp) { return tp->surface() && tp->surface()->client() == token()->creator();});
        }
    }

    if (!allowed)
        token()->destroy();

    destroyTokensOlderThanMs(10000);
}
//! [createTokenRequest]

//! [activateSurfaceRequest]
void LActivationTokenManager::activateSurfaceRequest(LSurface *surface)
{
    bool allowed { false };

    if (token()->creator() && token()->triggeringEvent() && sessionLockManager()->state() == LSessionLockManager::Unlocked)
    {
        if (token()->triggeringEvent()->isPointerEvent())
            allowed = seat()->pointer()->focus() && seat()->pointer()->focus()->client() == token()->creator();
        else if (token()->triggeringEvent()->isKeyboardEvent())
            allowed = seat()->keyboard()->focus() && seat()->keyboard()->focus()->client() == token()->creator();
        else if (token()->triggeringEvent()->isTouchEvent())
        {
            const auto &touchPoints { seat()->touch()->touchPoints() };
            allowed = std::any_of(touchPoints.begin(), touchPoints.end(), [this](LTouchPoint *tp) { return tp->surface() && tp->surface()->client() == token()->creator();});
        }
    }

    if (allowed)
    {
        if (surface->toplevel())
            surface->toplevel()->activateRequest();
        else
        {
            seat()->keyboard()->setFocus(surface);
        }
    }

    token()->destroy();
    destroyTokensOlderThanMs(10000);
}
//! [activateSurfaceRequest]
