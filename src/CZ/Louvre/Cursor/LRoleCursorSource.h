#ifndef LROLECURSORSOURCE_H
#define LROLECURSORSOURCE_H

#include <CZ/Louvre/Cursor/LCursorSource.h>
#include <CZ/skia/core/SkPoint.h>
#include <CZ/Core/Events/CZEvent.h>
#include <CZ/Core/CZWeak.h>

class CZ::LRoleCursorSource : public LCursorSource
{
public:
    std::shared_ptr<RImage> image() const noexcept override;
    SkIPoint hotspot() const noexcept override;
    Visibility visibility() const noexcept override;

    // Guaranteed to be valid at request time
    LClient *client() const noexcept override;
    const CZEvent *triggeringEvent() const noexcept override;
    LSurface *cursorSurface() const noexcept { return m_surface; };
protected:
    friend class LCursor;
    friend class LClient;
    friend class LCursorRole;
    friend class Protocols::Wayland::RPointer;
    static std::shared_ptr<LRoleCursorSource> MakeDefault(LClient *client) noexcept;
    LRoleCursorSource() noexcept : LCursorSource(Role) {};
    void onEnter(LOutput *output) noexcept override;
    void onLeave(LOutput *output) noexcept override;
    std::shared_ptr<CZEvent> m_triggeringEvent;
    Visibility m_visibility { Visible };
    CZWeak<LSurface> m_surface;
    CZWeak<LClient> m_client;
    std::shared_ptr<RImage> m_image;
    SkIPoint m_hotspot {};
};

#endif // LROLECURSORSOURCE_H
