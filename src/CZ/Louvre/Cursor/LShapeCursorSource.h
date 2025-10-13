#ifndef LSHAPECURSORSOURCE_H
#define LSHAPECURSORSOURCE_H

#include <CZ/Louvre/Cursor/LCursorSource.h>
#include <CZ/Core/CZCursorShape.h>
#include <CZ/skia/core/SkPoint.h>
#include <CZ/Core/CZWeak.h>

class CZ::LShapeCursorSource : public LCursorSource
{
public:
    std::shared_ptr<RImage> image() const noexcept override;
    SkIPoint hotspot() const noexcept override;
    Visibility visibility() const noexcept override;

    // Guaranteed to be valid at request time
    LClient *client() const noexcept override;
    const CZEvent *triggeringEvent() const noexcept override;

    CZCursorShape shape() const noexcept { return m_shape; }
protected:
    friend class LCursor;
    friend class LCursorSource;
    friend class Protocols::CursorShape::RCursorShapeDevice;
    static std::shared_ptr<LShapeCursorSource> MakeClient(
        CZCursorShape shape, LClient *client, std::shared_ptr<CZEvent> triggeringEvent) noexcept;
    LShapeCursorSource(CZCursorShape shape, Visibility visibility, LClient *client, std::shared_ptr<CZEvent> triggeringEvent) noexcept;
    CZCursorShape m_shape;
    Visibility m_visibility;
    CZWeak<LClient> m_client;
    std::shared_ptr<CZEvent> m_triggeringEvent;
};

#endif // LSHAPECURSORSOURCE_H
