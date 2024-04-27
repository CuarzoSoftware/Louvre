#include <LTouchPoint.h>
#include <LTouch.h>
#include <LSeat.h>
#include <LPointer.h>
#include <LOutput.h>
#include <cassert>

using namespace Louvre;

LTouch::LTouch(const void *params)
{
    assert(params != nullptr && "Invalid parameter passed to LTouch() constructor. LTouch can only be created from LCompositor::createTouchRequest().");
    LTouch **ptr { (LTouch**) params };
    assert(*ptr == nullptr && *ptr == seat()->touch() && "Only a single LTouch() instance can exist.");
    *ptr = this;
}

LTouch::~LTouch()
{
    while (!touchPoints().empty())
    {
        delete m_touchPoints.back();
        m_touchPoints.pop_back();
    }
}

LSurface *LTouch::surfaceAt(const LPoint &point)
{
    return seat()->pointer()->surfaceAt(point);
}

const std::vector<LTouchPoint*> &LTouch::touchPoints() const
{
    return m_touchPoints;
}

LTouchPoint *LTouch::createOrGetTouchPoint(const LTouchDownEvent &event)
{
    for (LTouchPoint *tp : touchPoints())
        if (tp->id() == event.id())
            return tp;

    return new LTouchPoint(event);
}

LTouchPoint *LTouch::findTouchPoint(Int32 id) const
{
    for (LTouchPoint *tp : touchPoints())
        if (tp->id() == id)
            return tp;

    return nullptr;
}

LPointF LTouch::toGlobal(LOutput *output, const LPointF &touchPointPos)
{
    if (!output)
        return touchPointPos;

    LPointF point;

    switch (output->transform())
    {
    case LTransform::Normal:
        point = output->size() * touchPointPos;
        break;
    case LTransform::Rotated270:
        point.setX(output->size().w() * touchPointPos.y());
        point.setY(output->size().h() * (1.f - touchPointPos.x()));
        break;
    case LTransform::Rotated90:
        point.setX(output->size().w() * (1.f - touchPointPos.y()));
        point.setY(output->size().h() * touchPointPos.x());
        break;
    case LTransform::Rotated180:
        point.setX(output->size().w() * (1.f - touchPointPos.x()));
        point.setY(output->size().h() * (1.f - touchPointPos.y()));
        break;
    case LTransform::Flipped180:
        point.setX(output->size().w() * touchPointPos.x());
        point.setY(output->size().h() * (1.f - touchPointPos.y()));
        break;
    case LTransform::Flipped:
        point.setX(output->size().w() * (1.f - touchPointPos.x()));
        point.setY(output->size().h() * touchPointPos.y());
        break;
    case LTransform::Flipped270:
        point.setX(output->size().w() * (1.f - touchPointPos.y()));
        point.setY(output->size().h() * (1.f - touchPointPos.x()));
        break;
    case LTransform::Flipped90:
        point.setX(output->size().w() * touchPointPos.y());
        point.setY(output->size().h() * touchPointPos.x());
        break;
    }

    return point + output->pos();
}

void LTouch::sendFrameEvent(const LTouchFrameEvent &event)
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

void LTouch::sendCancelEvent(const LTouchCancelEvent &event)
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
