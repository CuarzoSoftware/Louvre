#include <LRegion.h>
#include <pixman.h>

using namespace Louvre;

LRegion::LRegion()
{
    pixman_region32_init(&m_region);
}

LRegion::LRegion(const LRect &rect)
{
    pixman_region32_init_rect(&m_region, rect.x(), rect.y(), rect.w(), rect.h());
}

LRegion::~LRegion()
{
    pixman_region32_fini(&m_region);
}

LRegion::LRegion(const LRegion &other)
{
    pixman_region32_init(&m_region);
    pixman_region32_copy(&m_region, &other.m_region);
}

Louvre::LRegion &LRegion::operator=(const LRegion &other)
{
    pixman_region32_copy(&m_region, &other.m_region);
    return *this;
}

void LRegion::clear()
{
    pixman_region32_clear(&m_region);
}

void LRegion::addRect(const LRect &rect)
{
    pixman_region32_union_rect(&m_region, &m_region, rect.x(), rect.y(), rect.w(), rect.h());
}

void LRegion::addRect(const LPoint &pos, const LSize &size)
{
    pixman_region32_union_rect(&m_region, &m_region, pos.x(), pos.y(), size.w(), size.h());
}

void LRegion::addRect(Int32 x, Int32 y, const LSize &size)
{
    pixman_region32_union_rect(&m_region, &m_region, x, y, size.w(), size.h());
}

void LRegion::addRect(const LPoint &pos, Int32 w, Int32 h)
{
    pixman_region32_union_rect(&m_region, &m_region, pos.x(), pos.y(), w, h);
}

void LRegion::addRect(Int32 x, Int32 y, Int32 w, Int32 h)
{
    pixman_region32_union_rect(&m_region, &m_region, x, y, w, h);
}

void LRegion::addRegion(const LRegion &region)
{
    pixman_region32_union(&m_region, &m_region, &region.m_region);
}

void LRegion::subtractRect(const LRect &rect)
{
    pixman_region32_t tmp;
    pixman_region32_init_rect(&tmp, rect.x(), rect.y(), rect.w(), rect.h());
    pixman_region32_subtract(&m_region, &m_region, &tmp);
    pixman_region32_fini(&tmp);
}

void LRegion::subtractRect(const LPoint &pos, const LSize &size)
{
    pixman_region32_t tmp;
    pixman_region32_init_rect(&tmp, pos.x(), pos.y(), size.w(), size.h());
    pixman_region32_subtract(&m_region, &m_region, &tmp);
    pixman_region32_fini(&tmp);
}

void LRegion::subtractRect(const LPoint &pos, Int32 w, Int32 h)
{
    pixman_region32_t tmp;
    pixman_region32_init_rect(&tmp, pos.x(), pos.y(), w, h);
    pixman_region32_subtract(&m_region, &m_region, &tmp);
    pixman_region32_fini(&tmp);
}

void LRegion::subtractRect(Int32 x, Int32 y, const LSize &size)
{
    pixman_region32_t tmp;
    pixman_region32_init_rect(&tmp, x, y, size.w(), size.h());
    pixman_region32_subtract(&m_region, &m_region, &tmp);
    pixman_region32_fini(&tmp);
}

void LRegion::subtractRect(Int32 x, Int32 y, Int32 w, Int32 h)
{
    pixman_region32_t tmp;
    pixman_region32_init_rect(&tmp, x, y, w, h);
    pixman_region32_subtract(&m_region, &m_region, &tmp);
    pixman_region32_fini(&tmp);
}

void LRegion::subtractRegion(const LRegion &region)
{
    pixman_region32_subtract(&m_region, &m_region, &region.m_region);
}

void LRegion::intersectRegion(const LRegion &region)
{
    pixman_region32_intersect(&m_region, &m_region, &region.m_region);
}

void LRegion::multiply(Float32 factor)
{
    if (factor == 1.f)
        return;

    pixman_region32_t tmp;
    pixman_region32_init(&tmp);

    int n;
    pixman_box32_t *rects = pixman_region32_rectangles(&m_region, &n);

    if (factor == 0.5f)
    {
        for (int i = 0; i < n; i++)
        {
            pixman_region32_union_rect(
                        &tmp,
                        &tmp,
                        rects->x1 >> 1,
                        rects->y1 >> 1,
                        (rects->x2 - rects->x1) >> 1,
                        (rects->y2 - rects->y1) >> 1);
            rects++;
        }
    }
    else if (factor == 2.f)
    {
        for (int i = 0; i < n; i++)
        {
            pixman_region32_union_rect(
                        &tmp,
                        &tmp,
                        rects->x1 << 1,
                        rects->y1 << 1,
                        (rects->x2 - rects->x1) << 1,
                        (rects->y2 - rects->y1) << 1);
            rects++;
        }
    }
    else
    {
        for (int i = 0; i < n; i++)
        {
            pixman_region32_union_rect(
                &tmp,
                &tmp,
                floorf(float(rects->x1) * factor),
                floorf(float(rects->y1) * factor),
                ceilf(float(rects->x2 - rects->x1) * factor),
                ceilf(float(rects->y2 - rects->y1) * factor));
            rects++;
        }
    }

    pixman_region32_fini(&m_region);
    m_region = tmp;
}

void LRegion::multiply(Float32 xFactor, Float32 yFactor)
{
    if (xFactor == 1.f && yFactor == 1.f)
        return;

    pixman_region32_t tmp;
    pixman_region32_init(&tmp);

    int n;
    pixman_box32_t *rects = pixman_region32_rectangles(&m_region, &n);

    for (int i = 0; i < n; i++)
    {
        pixman_region32_union_rect(
            &tmp,
            &tmp,
            floor(float(rects->x1) * xFactor),
            floor(float(rects->y1) * yFactor),
            ceil(float(rects->x2 - rects->x1) * xFactor),
            ceil(float(rects->y2 - rects->y1) * yFactor));
        rects++;
    }

    pixman_region32_fini(&m_region);
    m_region = tmp;
}

bool LRegion::containsPoint(const LPoint &point) const
{
    return pixman_region32_contains_point(&m_region, point.x(), point.y(), NULL);
}

void LRegion::offset(const LPoint &offset)
{
    if (offset.x() == 0 && offset.y() == 0)
        return;

    pixman_region32_translate(&m_region, offset.x(), offset.y());
}

void LRegion::offset(Int32 x, Int32 y)
{
    if (x == 0 && x == 0)
        return;

    pixman_region32_translate(&m_region, x, y);
}

void LRegion::inverse(const LRect &rect)
{
    pixman_box32_t r;
    r.x1 = rect.x();
    r.x2 = r.x1 + rect.w();
    r.y1 = rect.y();
    r.y2 = r.y1 + rect.h();

    pixman_region32_inverse(&m_region, &m_region, &r);
}

bool LRegion::empty() const
{
    return !pixman_region32_not_empty(&m_region);
}

void LRegion::clip(const LRect &rect)
{
    pixman_region32_intersect_rect(&m_region, &m_region, rect.x(), rect.y(), rect.w(), rect.h());
}

void LRegion::clip(const LPoint &pos, const LSize &size)
{
    pixman_region32_intersect_rect(&m_region, &m_region, pos.x(), pos.y(), size.w(), size.h());
}

void LRegion::clip(Int32 x, Int32 y, Int32 w, Int32 h)
{
    pixman_region32_intersect_rect(&m_region, &m_region, x, y, w, h);
}

const LBox &LRegion::extents() const
{
    return *(LBox*)pixman_region32_extents(&m_region);
}

LBox *LRegion::boxes(Int32 *n) const
{
    return (LBox*)pixman_region32_rectangles(&m_region, n);
}

void LRegion::transform(const LSize &size, LFramebuffer::Transform transform)
{
    pixman_region32_intersect_rect(&m_region, &m_region, 0, 0, size.w(), size.h());

    LBox *boxes;
    Int32 n, i;
    pixman_region32_t tmp;

    switch (transform)
    {
    case LFramebuffer::Normal:
        return;
    case LFramebuffer::Flipped270:
        pixman_region32_init(&tmp);
        boxes = (LBox*)pixman_region32_rectangles(&m_region, &n);
        for (i = 0; i < n; i++)
        {
            pixman_region32_union_rect(&tmp, &tmp,
                                       size.h() - boxes->y2,
                                       size.w() - boxes->x2,
                                       boxes->y2 - boxes->y1,
                                       boxes->x2 - boxes->x1);
            boxes++;
        }
        break;
    case LFramebuffer::Flipped90:
        pixman_region32_init(&tmp);
        boxes = (LBox*)pixman_region32_rectangles(&m_region, &n);
        for (i = 0; i < n; i++)
        {
            pixman_region32_union_rect(&tmp, &tmp,
                                       boxes->y1,
                                       boxes->x1,
                                       boxes->y2 - boxes->y1,
                                       boxes->x2 - boxes->x1);
            boxes++;
        }
        break;
    case LFramebuffer::Flipped180:
        pixman_region32_init(&tmp);
        boxes = (LBox*)pixman_region32_rectangles(&m_region, &n);
        for (i = 0; i < n; i++)
        {
            pixman_region32_union_rect(&tmp, &tmp,
                                       boxes->x1,
                                       size.h() - boxes->y2,
                                       boxes->x2 - boxes->x1,
                                       boxes->y2 - boxes->y1);
            boxes++;
        }
        break;
    case LFramebuffer::Rotated180:
        pixman_region32_init(&tmp);
        boxes = (LBox*)pixman_region32_rectangles(&m_region, &n);
        for (i = 0; i < n; i++)
        {
            pixman_region32_union_rect(&tmp, &tmp,
                                       size.w() - boxes->x2,
                                       size.h() - boxes->y2,
                                       boxes->x2 - boxes->x1,
                                       boxes->y2 - boxes->y1);
            boxes++;
        }
        break;
    case LFramebuffer::Flipped:
        pixman_region32_init(&tmp);
        boxes = (LBox*)pixman_region32_rectangles(&m_region, &n);
        for (i = 0; i < n; i++)
        {
            pixman_region32_union_rect(&tmp, &tmp,
                                       size.w() - boxes->x2,
                                       boxes->y1,
                                       boxes->x2 - boxes->x1,
                                       boxes->y2 - boxes->y1);
            boxes++;
        }
        break;
    case LFramebuffer::Rotated90:
        pixman_region32_init(&tmp);
        boxes = (LBox*)pixman_region32_rectangles(&m_region, &n);
        for (i = 0; i < n; i++)
        {
            pixman_region32_union_rect(&tmp, &tmp,
                                       boxes->y1,
                                       size.w() - boxes->x2,
                                       boxes->y2 - boxes->y1,
                                       boxes->x2 - boxes->x1);
            boxes++;
        }
        break;
    case LFramebuffer::Rotated270:
        pixman_region32_init(&tmp);
        boxes = (LBox*)pixman_region32_rectangles(&m_region, &n);
        for (i = 0; i < n; i++)
        {
            pixman_region32_union_rect(&tmp, &tmp,
                                       size.h() - boxes->y2,
                                       boxes->x1,
                                       boxes->y2 - boxes->y1,
                                       boxes->x2 - boxes->x1);
            boxes++;
        }
        break;
    default:
        return;
    }

    pixman_region32_fini(&m_region);
    m_region = tmp;
}

void LRegion::multiply(LRegion *dst, LRegion *src, Float32 factor)
{
    if (dst == src)
    {
        src->multiply(factor);
        return;
    }

    if (factor == 1.f)
    {
        *dst = *src;
        return;
    }

    pixman_region32_clear(&dst->m_region);

    int n;
    pixman_box32_t *rects = pixman_region32_rectangles(&src->m_region, &n);

    if (factor == 0.5f)
    {
        for (int i = 0; i < n; i++)
        {
            pixman_region32_union_rect(
                &dst->m_region,
                &dst->m_region,
                rects->x1 >> 1,
                rects->y1 >> 1,
                (rects->x2 - rects->x1) >> 1,
                (rects->y2 - rects->y1) >> 1);
            rects++;
        }
    }
    else if (factor == 2.f)
    {
        for (int i = 0; i < n; i++)
        {
            pixman_region32_union_rect(
                &dst->m_region,
                &dst->m_region,
                rects->x1 << 1,
                rects->y1 << 1,
                (rects->x2 - rects->x1) << 1,
                (rects->y2 - rects->y1) << 1);
            rects++;
        }
    }
    else
    {
        for (int i = 0; i < n; i++)
        {
            pixman_region32_union_rect(
                &dst->m_region,
                &dst->m_region,
                floor(float(rects->x1) * factor),
                floor(float(rects->y1) * factor),
                ceil(float(rects->x2 - rects->x1) * factor),
                ceil(float(rects->y2 - rects->y1) * factor));
            rects++;
        }
    }
}
