#include <CZ/Louvre/Seat/LClipboard.h>
#include <CZ/Core/Events/CZEvent.h>
#include <CZ/Louvre/Seat/LSeat.h>
#include <CZ/Louvre/Seat/LPointer.h>
#include <CZ/Louvre/Seat/LKeyboard.h>
#include <CZ/Louvre/Seat/LTouch.h>
#include <CZ/Louvre/Seat/LTouchPoint.h>

using namespace CZ;

//! [setClipboardRequest]
bool LClipboard::setClipboardRequest(LClient *client, const CZEvent &triggeringEvent)
{
    if (triggeringEvent.isPointerEvent())
        return !seat()->pointer()->focus() || seat()->pointer()->focus()->client() == client;
    else if (triggeringEvent.isKeyboardEvent())
        return !seat()->keyboard()->focus() || seat()->keyboard()->focus()->client() == client;
    else if (triggeringEvent.isTouchEvent())
        for (LTouchPoint *tp : seat()->touch()->touchPoints())
            if (tp->surface() && tp->surface()->client() == client)
                return true;

    return false;
}
//! [setClipboardRequest]

//! [persistentMimeTypeFilter]
bool LClipboard::persistentMimeTypeFilter(const std::string &mimeType) const
{
    return
        mimeType == "image/png" ||
        mimeType == "text/uri-list" ||
        mimeType == "UTF8_STRING" ||
        mimeType == "COMPOUND_TEXT" ||
        mimeType == "TEXT" ||
        mimeType == "STRING" ||
        mimeType == "text/plain;charset=utf-8" ||
        mimeType == "text/plain";
}
//! [persistentMimeTypeFilter]
