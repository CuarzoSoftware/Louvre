#include <CZ/Louvre/Cursor/LImageCursorSource.h>
#include <CZ/Louvre/Cursor/LShapeCursorSource.h>
#include <CZ/Louvre/Cursor/LRoleCursorSource.h>
#include <CZ/Louvre/LLog.h>

#include <CZ/Ream/RCore.h>
#include <CZ/Ream/RImage.h>
#include <CZ/Ream/RDevice.h>

#include <CZ/Core/CZBitset.h>

#include <X11/Xcursor/Xcursor.h>

using namespace CZ;

std::shared_ptr<LImageCursorSource> LCursorSource::Make(std::shared_ptr<RImage> image, SkIPoint hotspot) noexcept
{
    if (!image)
    {
        LLog(CZError, CZLN, "Invalid RImage (nullptr)");
        return {};
    }

    if (image->checkDeviceCaps(RImageCap_Src, RCore::Get()->mainDevice()).get() != RImageCap_Src)
    {
        LLog(CZError, CZLN, "Invalid RImage (missing RImageCap_Src for the main device)");
        return {};
    }

    auto source { std::shared_ptr<LImageCursorSource>(new LImageCursorSource(image, hotspot)) };
    source->m_self = source;
    return source;
}

std::shared_ptr<LImageCursorSource> LCursorSource::MakeFromTheme(const char *name, const char *theme, Int32 suggestedSize) noexcept
{
    if (!name)
    {
        LLog(CZError, CZLN, "Invalid cursor name (nullptr)");
        return {};
    }

    auto ream { RCore::Get() };

    if (!ream)
    {
        LLog(CZError, CZLN, "Cursors must be created after the compositor is initialized");
        return  {};
    }

    const auto &formats { ream->mainDevice()->textureFormats().formats() };
    auto format { formats.find(DRM_FORMAT_ABGR8888) };

    if (format == formats.end())
        return {};

    auto *xCursor { XcursorLibraryLoadImage(name, theme, suggestedSize) };

    if (!xCursor)
    {
        LLog(CZError, CZLN, "Failed to load X Cursor {}", name);
        return {};
    }

    const auto hotspot { SkIPoint::Make(xCursor->xhot, xCursor->yhot) };

    RPixelBufferInfo info {};
    info.pixels = (UInt8*)xCursor->pixels;
    info.alphaType = kPremul_SkAlphaType;
    info.format = DRM_FORMAT_ABGR8888;
    info.stride = xCursor->width * 4;
    info.size = SkISize::Make(xCursor->width, xCursor->height);

    auto image { RImage::MakeFromPixels(info, *format) };
    XcursorImageDestroy(xCursor);

    if (!image)
    {
        LLog(CZError, CZLN, "Failed to create image from X Cursor");
        return {};
    }

    auto source { std::shared_ptr<LImageCursorSource>(new LImageCursorSource(image, hotspot)) };
    source->m_self = source;
    return source;
}

std::shared_ptr<LShapeCursorSource> LCursorSource::MakeShape(CZCursorShape shape) noexcept
{
    auto source { std::shared_ptr<LShapeCursorSource>(new LShapeCursorSource(shape, Auto, nullptr, nullptr)) };
    source->m_self = source;
    return source;
}

std::shared_ptr<LImageCursorSource> LCursorSource::asImage() noexcept
{
    return std::dynamic_pointer_cast<LImageCursorSource>(m_self.lock());
}

std::shared_ptr<LRoleCursorSource> LCursorSource::asRole() noexcept
{
    return std::dynamic_pointer_cast<LRoleCursorSource>(m_self.lock());
}

std::shared_ptr<LShapeCursorSource> LCursorSource::asShape() noexcept
{
    return std::dynamic_pointer_cast<LShapeCursorSource>(m_self.lock());
}
