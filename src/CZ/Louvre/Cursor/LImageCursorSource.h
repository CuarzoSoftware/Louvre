#ifndef CZ_LCURSORIMAGE_H
#define CZ_LCURSORIMAGE_H

#include <CZ/Louvre/Cursor/LCursorSource.h>
#include <CZ/skia/core/SkPoint.h>

class CZ::LImageCursorSource : public LCursorSource
{
public:
    std::shared_ptr<RImage> image() const noexcept override { return m_image; };
    SkIPoint hotspot() const noexcept override { return m_hotspot; };

protected:
    friend class LCursorSource;
    LImageCursorSource(std::shared_ptr<RImage> image, SkIPoint hotspot) noexcept :
        LCursorSource(Image), m_image(image), m_hotspot(hotspot) {}

    std::shared_ptr<RImage> m_image;
    SkIPoint m_hotspot;
};

#endif // CZ_LCURSORIMAGE_H
