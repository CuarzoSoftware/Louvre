#include <protocols/Wayland/private/RDataSourcePrivate.h>
#include <LClipboard.h>
#include <LEvent.h>
#include <LSeat.h>
#include <LPointer.h>
#include <LKeyboard.h>
#include <LTouch.h>
#include <LTouchPoint.h>
#include <cassert>

using namespace Louvre;

LClipboard::LClipboard(const void *params) noexcept
{
    assert(params != nullptr && "Invalid parameter passed to LClipboard() constructor. LClipboard can only be created from LCompositor::createClipboardRequest().");
    LClipboard**ptr { (LClipboard**) params };
    assert(*ptr == nullptr && *ptr == seat()->clipboard() && "Only a single LClipboard() instance can exist.");
    *ptr = this;
}

LClipboard::~LClipboard() noexcept
{
    clear();
}

bool LClipboard::setClipboardRequest(LClient *client, const LEvent *triggeringEvent) noexcept
{
    if (!triggeringEvent)
    {
        /* Request without valid triggering event */
        return false;
    }

    switch (triggeringEvent->type())
    {
    case LEvent::Type::Pointer:
        return !seat()->pointer()->focus() || seat()->pointer()->focus()->client() == client;
    case LEvent::Type::Keyboard:
        return !seat()->keyboard()->focus() || seat()->keyboard()->focus()->client() == client;
    case LEvent::Type::Touch:
        for (LTouchPoint *tp : seat()->touch()->touchPoints())
            if (tp->surface() && tp->surface()->client() == client)
                return true;
        break;
    }

    return false;
}

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

const std::vector<Protocols::Wayland::RDataSource::MimeTypeFile> &LClipboard::mimeTypes() const noexcept
{
    if (m_dataSource.get())
        return m_dataSource.get()->imp()->mimeTypes;

    return m_persistentMimeTypes;
}

void LClipboard::clear()
{
    while (!m_persistentMimeTypes.empty())
    {
        fclose(m_persistentMimeTypes.back().tmp);
        m_persistentMimeTypes.pop_back();
    }
}
