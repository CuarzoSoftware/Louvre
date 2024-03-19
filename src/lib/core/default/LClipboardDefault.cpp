#include <LClipboard.h>
#include <LEvent.h>
#include <LSeat.h>
#include <LPointer.h>
#include <LKeyboard.h>
#include <LTouch.h>
#include <LTouchPoint.h>

using namespace Louvre;

//! [setClipboardRequest]
bool LClipboard::setClipboardRequest(LClient *client, const LEvent &triggeringEvent) noexcept
{
    switch (triggeringEvent.type())
    {
    case LEvent::Type::Pointer:
        return !seat()->pointer()->focus() || seat()->pointer()->focus()->client() == client;
    case LEvent::Type::Keyboard:
        return !seat()->keyboard()->focus() || seat()->keyboard()->focus()->client() == client;
    case LEvent::Type::Touch:
        for (LTouchPoint *tp : seat()->touch()->touchPoints())
            if (tp->surface() && tp->surface()->client() == client)
                return true;
    }

    return false;
}
//! [setClipboardRequest]

//! [persistentMimeTypeFilter]
bool LClipboard::persistentMimeTypeFilter(const std::string &mimeType) const noexcept
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
