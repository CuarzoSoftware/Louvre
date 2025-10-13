#include <CZ/Louvre/Cursor/LShapeCursorSource.h>
#include <CZ/Louvre/Cursor/LImageCursorSource.h>
#include <CZ/Louvre/LClient.h>
#include <CZ/Louvre/Cursor/LCursor.h>
#include <CZ/Louvre/LLog.h>

using namespace CZ;

std::shared_ptr<RImage> LShapeCursorSource::image() const noexcept
{
    if (!cursor())
    {
        LLog(CZWarning, CZLN, "Cursor source used without an available LCursor");
        return {};
    }

    if (auto shape = cursor()->getShapeAsset(m_shape))
        return shape->image();

    return cursor()->fallbackSource()->image();
}

SkIPoint LShapeCursorSource::hotspot() const noexcept
{
    if (!cursor())
    {
        LLog(CZWarning, CZLN, "Cursor source used without an available LCursor");
        return {};
    }

    if (auto shape = cursor()->getShapeAsset(m_shape))
        return shape->hotspot();

    return cursor()->fallbackSource()->hotspot();
}

LClient *LShapeCursorSource::client() const noexcept
{
    return m_client;
}

const CZEvent *LShapeCursorSource::triggeringEvent() const noexcept
{
    return m_triggeringEvent.get();
}

LShapeCursorSource::LShapeCursorSource(CZCursorShape shape, Visibility visibility, LClient *client, std::shared_ptr<CZEvent> triggeringEvent) noexcept
    : LCursorSource(Shape), m_shape(shape), m_visibility(visibility), m_client(client), m_triggeringEvent(triggeringEvent) {}

std::shared_ptr<CZ::LShapeCursorSource> LShapeCursorSource::MakeClient(CZCursorShape shape, LClient *client, std::shared_ptr<CZEvent> triggeringEvent) noexcept
{
    auto source { std::shared_ptr<LShapeCursorSource>(new LShapeCursorSource(shape, Visible, client, triggeringEvent)) };
    source->m_self = source;
    return source;
}

LCursorSource::Visibility LShapeCursorSource::visibility() const noexcept
{
    return m_visibility;
}
