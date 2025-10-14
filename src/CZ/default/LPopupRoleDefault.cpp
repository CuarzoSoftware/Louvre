#include <CZ/Louvre/Protocols/Wayland/GSeat.h>
#include <CZ/Louvre/Private/LPopupRolePrivate.h>
#include <CZ/Louvre/Private/LSurfacePrivate.h>
#include <CZ/Louvre/Manager/LSessionLockManager.h>
#include <CZ/Core/Events/CZTouchDownEvent.h>
#include <CZ/Louvre/Seat/LTouchPoint.h>
#include <CZ/Louvre/Seat/LTouch.h>
#include <CZ/Louvre/Roles/LPositioner.h>
#include <CZ/Louvre/Seat/LSeat.h>
#include <CZ/Louvre/Seat/LKeyboard.h>
#include <CZ/Louvre/LCompositor.h>
#include <CZ/Louvre/Seat/LPointer.h>
#include <CZ/Louvre/Cursor/LCursor.h>
#include <CZ/Louvre/Seat/LOutput.h>

using namespace CZ;

//! [rolePos]
SkIPoint LPopupRole::rolePos() const
{
    /* Just in case this method is called while the popup is being destroyed */

    if (!surface()->parent())
        return m_rolePos;

    m_rolePos = surface()->parent()->rolePos() + localPos() - windowGeometry().topLeft();

    if (auto *p = surface()->parent()->toplevel())
        m_rolePos += p->windowGeometry().topLeft();
    else if (auto *p = surface()->parent()->popup())
        m_rolePos += p->windowGeometry().topLeft();

    return m_rolePos;
}
//! [rolePos]

//! [grabKeyboardRequest]
void LPopupRole::grabKeyboardRequest(const CZEvent &triggeringEvent)
{
    if (sessionLockManager()->state() != LSessionLockManager::Unlocked && sessionLockManager()->client() != client())
        return;

    if (triggeringEvent.isPointerEvent())
    {
        if (!seat()->pointer()->focus())
            return;

        if (seat()->pointer()->focus()->client() != surface()->client())
            return;
    }
    else if (triggeringEvent.isKeyboardEvent())
    {
        if (!seat()->keyboard()->focus())
            return;

        if (seat()->keyboard()->focus()->client() != surface()->client())
            return;
    }
    else if (triggeringEvent.isTouchEvent())
    {
        if (!triggeringEvent.typeIsAnyOf(CZEvent::Type::TouchDown))
            return;

        const auto &touchDownEvent { (const CZTouchDownEvent &)triggeringEvent };
        const auto *touchPoint { seat()->touch()->findTouchPoint(touchDownEvent.id) };

        if (!touchPoint->surface())
            return;

        if (touchPoint->surface()->client() != surface()->client())
            return;
    }

    seat()->keyboard()->setGrab(surface());
}
//! [grabKeyboardRequest]

//! [atomsChanged]
void LPopupRole::stateChanged(CZBitset<Changes> changes, const State &prev)
{
    CZ_UNUSED(changes)
    CZ_UNUSED(prev)
}
//! [atomsChanged]

//! [configureRequest]
void LPopupRole::configureRequest()
{
    // Ensure the Popup stays within the boundaries of the current output where the cursor is positioned
    setBounds(cursor()->output() != nullptr ? cursor()->output()->rect() : SkIRect::MakeEmpty());
    configureRect(calculateUnconstrainedRect());
}
//! [configureRequest]
