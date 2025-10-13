#include <CZ/Louvre/Cursor/LRoleCursorSource.h>
#include <CZ/Louvre/Cursor/LImageCursorSource.h>
#include <CZ/Core/Events/CZPointerEnterEvent.h>
#include <CZ/Louvre/LClient.h>
#include <CZ/Louvre/Roles/LSurface.h>
#include <CZ/Louvre/Cursor/LCursor.h>

using namespace CZ;

std::shared_ptr<RImage> LRoleCursorSource::image() const noexcept
{
    return m_image;
}

SkIPoint LRoleCursorSource::hotspot() const noexcept
{
    return m_hotspot;
}

LClient *LRoleCursorSource::client() const noexcept
{
    return m_client;
}

const CZEvent *LRoleCursorSource::triggeringEvent() const noexcept
{
    return m_triggeringEvent.get();
}

std::shared_ptr<LRoleCursorSource> LRoleCursorSource::MakeDefault(LClient *client) noexcept
{
    assert(client);
    auto source { std::shared_ptr<LRoleCursorSource>(new LRoleCursorSource()) };
    source->m_self = source;
    source->m_client = client;
    source->m_triggeringEvent.reset(new CZPointerEnterEvent());
    source->m_triggeringEvent->serial = 0;
    source->m_hotspot = cursor()->fallbackSource()->hotspot();
    source->m_image = cursor()->fallbackSource()->image();
    return source;
}

void LRoleCursorSource::onEnter(LOutput *output) noexcept
{
    if (m_surface)
        m_surface->sendOutputEnterEvent(output);
}

void LRoleCursorSource::onLeave(LOutput *output) noexcept
{
    if (m_surface)
        m_surface->sendOutputLeaveEvent(output);
}

LCursorSource::Visibility LRoleCursorSource::visibility() const noexcept
{
    return m_visibility;
}
