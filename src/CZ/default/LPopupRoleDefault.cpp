#include <CZ/Louvre/Protocols/Wayland/GSeat.h>
#include <CZ/Louvre/Private/LPopupRolePrivate.h>
#include <CZ/Louvre/Private/LSurfacePrivate.h>
#include <CZ/Louvre/LSessionLockManager.h>
#include <CZ/Louvre/Events/LTouchDownEvent.h>
#include <CZ/Louvre/LTouchPoint.h>
#include <CZ/Louvre/LTouch.h>
#include <CZ/Louvre/Roles/LPositioner.h>
#include <CZ/Louvre/LSeat.h>
#include <CZ/Louvre/LKeyboard.h>
#include <CZ/Louvre/LCompositor.h>
#include <CZ/Louvre/LPointer.h>
#include <CZ/Louvre/LCursor.h>
#include <CZ/Louvre/LOutput.h>

using namespace Louvre;

//! [rolePos]
SkIPoint LPopupRole::rolePos() const
{
    /* Just in case this method is called while the popup is being destroyed */
    if (!surface()->parent())
        return m_rolePos;

    m_rolePos = surface()->parent()->rolePos() + localPos() - windowGeometry().topLeft();

    if (surface()->parent()->toplevel())
        m_rolePos += surface()->parent()->toplevel()->windowGeometry().topLeft();
    else if (surface()->parent()->popup())
        m_rolePos += surface()->parent()->popup()->windowGeometry().topLeft();

    return m_rolePos;
}
//! [rolePos]

//! [grabKeyboardRequest]
void LPopupRole::grabKeyboardRequest(const LEvent &triggeringEvent)
{
    if (sessionLockManager()->state() != LSessionLockManager::Unlocked && sessionLockManager()->client() != client())
        return;

    if (triggeringEvent.type() == LEvent::Type::Pointer)
    {
        if (!seat()->pointer()->focus())
            return;

        if (seat()->pointer()->focus()->client() != surface()->client())
            return;
    }
    else if (triggeringEvent.type() == LEvent::Type::Keyboard)
    {
        if (!seat()->keyboard()->focus())
            return;

        if (seat()->keyboard()->focus()->client() != surface()->client())
            return;
    }
    else if (triggeringEvent.type() == LEvent::Type::Touch)
    {
        if (triggeringEvent.subtype() != LEvent::Subtype::Down)
            return;

        const auto &touchDownEvent { (const LTouchDownEvent &)triggeringEvent };
        const auto *touchPoint { seat()->touch()->findTouchPoint(touchDownEvent.id()) };

        if (!touchPoint->surface())
            return;

        if (touchPoint->surface()->client() != surface()->client())
            return;
    }

    seat()->keyboard()->setGrab(surface());
}
//! [grabKeyboardRequest]

//! [atomsChanged]
void LPopupRole::atomsChanged(CZBitset<AtomChanges> changes, const Atoms &prevAtoms)
{
    L_UNUSED(changes)
    L_UNUSED(prevAtoms)
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
