#include <LActivationTokenManager.h>
#include <LSessionLockManager.h>
#include <LSeat.h>
#include <LPointer.h>
#include <LKeyboard.h>
#include <LTouch.h>
#include <LTouchPoint.h>
#include <algorithm>

using namespace Louvre;

//! [createTokenRequest]
void LActivationTokenManager::createTokenRequest()
{
    // Deny requests not originated from a recent user event

    bool allowed { false };

    if (token()->triggeringEvent())
    {
        switch (token()->triggeringEvent()->type())
        {
        case LEvent::Type::Pointer:
            allowed = seat()->pointer()->focus() && seat()->pointer()->focus()->client() == token()->creator();
            break;
        case LEvent::Type::Keyboard:
            allowed = seat()->keyboard()->focus() && seat()->keyboard()->focus()->client() == token()->creator();
            break;
        case LEvent::Type::Touch:
            const auto &touchPoints { seat()->touch()->touchPoints() };
            allowed = std::any_of(touchPoints.begin(), touchPoints.end(), [this](LTouchPoint *tp) { return tp->surface() && tp->surface()->client() == token()->creator();});
            break;
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
        switch (token()->triggeringEvent()->type())
        {
        case LEvent::Type::Pointer:
            allowed = seat()->pointer()->focus() && seat()->pointer()->focus()->client() == token()->creator();
            break;
        case LEvent::Type::Keyboard:
            allowed = seat()->keyboard()->focus() && seat()->keyboard()->focus()->client() == token()->creator();
            break;
        case LEvent::Type::Touch:
            const auto &touchPoints { seat()->touch()->touchPoints() };
            allowed = std::any_of(touchPoints.begin(), touchPoints.end(), [this](LTouchPoint *tp) { return tp->surface() && tp->surface()->client() == token()->creator();});
            break;
        }
    }

    if (allowed)
    {
        if (surface->toplevel())
            surface->toplevel()->activateRequest();
        else
        {
            seat()->keyboard()->setFocus(surface);
            surface->raise();
        }
    }

    token()->destroy();
    destroyTokensOlderThanMs(10000);
}
//! [activateSurfaceRequest]
