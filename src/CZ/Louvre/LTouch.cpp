#include <LTouchPoint.h>
#include <LTouch.h>
#include <LSeat.h>
#include <LPointer.h>
#include <LOutput.h>
#include <cassert>

using namespace Louvre;

LTouch::LTouch(const void *params) noexcept : LFactoryObject(FactoryObjectType)
{
    assert(params != nullptr && "Invalid parameter passed to LTouch constructor.");
    LTouch **ptr { (LTouch**) params };
    assert(*ptr == nullptr && *ptr == seat()->touch() && "Only a single LTouch instance can exist.");
    *ptr = this;
}

LTouch::~LTouch() noexcept
{
    notifyDestruction();

    while (!touchPoints().empty())
    {
        delete m_touchPoints.back();
        m_touchPoints.pop_back();
    }
}

LSurface *LTouch::surfaceAt(const SkIPoint &point) const noexcept
{
    return seat()->pointer()->surfaceAt(point);
}

const std::vector<LTouchPoint*> &LTouch::touchPoints() const noexcept
{
    return m_touchPoints;
}

LTouchPoint *LTouch::createOrGetTouchPoint(const LTouchDownEvent &event) noexcept
{
    for (LTouchPoint *tp : touchPoints())
        if (tp->id() == event.id())
            return tp;

    return new LTouchPoint(event);
}

LTouchPoint *LTouch::findTouchPoint(Int32 id) const noexcept
{
    for (LTouchPoint *tp : touchPoints())
        if (tp->id() == id)
            return tp;

    return nullptr;
}

SkPoint LTouch::toGlobal(LOutput *output, const SkPoint &touchPointPos) noexcept
{
    if (!output)
        return touchPointPos;

    SkPoint point { 0.f, 0.f };

    switch (output->transform())
    {
    case CZTransform::Normal:
        point.fX = output->size().fWidth * touchPointPos.fX;
        point.fY = output->size().fHeight * touchPointPos.fY;
        break;
    case CZTransform::Rotated270:
        point.fX =output->size().width() * touchPointPos.y();
        point.fY =output->size().height() * (1.f - touchPointPos.x());
        break;
    case CZTransform::Rotated90:
        point.fX =output->size().width() * (1.f - touchPointPos.y());
        point.fY =output->size().height() * touchPointPos.x();
        break;
    case CZTransform::Rotated180:
        point.fX =output->size().width() * (1.f - touchPointPos.x());
        point.fY =output->size().height() * (1.f - touchPointPos.y());
        break;
    case CZTransform::Flipped180:
        point.fX =output->size().width() * touchPointPos.x();
        point.fY =output->size().height() * (1.f - touchPointPos.y());
        break;
    case CZTransform::Flipped:
        point.fX =output->size().width() * (1.f - touchPointPos.x());
        point.fY =output->size().height() * touchPointPos.y();
        break;
    case CZTransform::Flipped270:
        point.fX =output->size().width() * (1.f - touchPointPos.y());
        point.fY =output->size().height() * (1.f - touchPointPos.x());
        break;
    case CZTransform::Flipped90:
        point.fX =output->size().width() * touchPointPos.y();
        point.fY =output->size().height() * touchPointPos.x();
        break;
    }

    return SkPoint::Make(
        point.x() + output->pos().x(),
        point.y() + output->pos().y());
}

void LTouch::sendFrameEvent(const LTouchFrameEvent &event) noexcept
{
    L_UNUSED(event);

    LTouchPoint *tp;

    for (auto it = m_touchPoints.begin(); it != m_touchPoints.end();)
    {
        tp = *it;
        tp->sendTouchFrameEvent();

        if (tp->pressed())
            it++;
        else
        {
            it = m_touchPoints.erase(it);
            delete tp;
        }
    }
}

void LTouch::sendCancelEvent(const LTouchCancelEvent &event) noexcept
{
    L_UNUSED(event);

    LTouchPoint *tp;
    while (!touchPoints().empty())
    {
        tp = touchPoints().back();
        tp->sendTouchCancelEvent();
        m_touchPoints.pop_back();
        delete tp;
    }
}
