#include <LRegion.h>

using namespace Louvre;

void LRegion::multiply(Float32 factor) noexcept
{
    if (factor == 1.f)
        return;

    pixman_region32_t tmp;
    pixman_region32_init(&tmp);

    int n;
    const pixman_box32_t *rects { pixman_region32_rectangles(&m_region, &n) };

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

void LRegion::multiply(Float32 xFactor, Float32 yFactor) noexcept
{
    if (xFactor == 1.f && yFactor == 1.f)
        return;

    pixman_region32_t tmp;
    pixman_region32_init(&tmp);

    int n;
    const pixman_box32_t *rects { pixman_region32_rectangles(&m_region, &n) };

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

void LRegion::transform(const LSize &size, LFramebuffer::Transform transform) noexcept
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

LPointF LRegion::closestPointFrom(const LPointF &point, Float32 margin) const noexcept
{
    if (empty())
        return point;

    Float32 smallestDistance { std::numeric_limits<Float32>::max() };
    LPointF closestPoint;
    Float32 distance;
    LPointF tmpPoint;

    Int32 n;
    const LBox *box = boxes(&n);

    UInt8 in;
    for (Int32 i = 0; i < n; i++)
    {
        in = 0;

        if (point.x() <= box->x1)
            tmpPoint.setX(Float32(box->x1) + margin);
        else if (point.x() >= box->x2)
            tmpPoint.setX(Float32(box->x2) - margin);
        else
        {
            in++;
            tmpPoint.setX(point.x());
        }

        if (point.y() <= box->y1)
            tmpPoint.setY(Float32(box->y1) + margin);
        else if (point.y() >= box->y2)
            tmpPoint.setY(Float32(box->y2) - margin);
        else
        {
            in++;
            tmpPoint.setY(point.y());
        }

        if (in == 2)
            return tmpPoint;

        distance = point.distanceFrom(tmpPoint);

        if (distance < smallestDistance)
        {
            smallestDistance = distance;
            closestPoint = tmpPoint;
        }

        box++;
    }

    return closestPoint;
}

void LRegion::multiply(LRegion *dst, LRegion *src, Float32 factor) noexcept
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
    const pixman_box32_t *rects { pixman_region32_rectangles(&src->m_region, &n) };

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
