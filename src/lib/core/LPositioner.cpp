#include <private/LPositionerPrivate.h>

LPositioner::LPositioner()
{
    m_imp = new LPositionerPrivate();
}

LPositioner::~LPositioner()
{
    delete m_imp;
}

const LSize &LPositioner::size() const
{
    return imp()->data.size;
}

const LRect &LPositioner::anchorRect() const
{
    return imp()->data.anchorRect;
}

const LPoint &LPositioner::offset() const
{
    return imp()->data.offset;
}

UInt32 LPositioner::anchor() const
{
    return imp()->data.anchor;
}

UInt32 LPositioner::gravity() const
{
    return imp()->data.gravity;
}

LPositioner::ConstraintAdjustments LPositioner::constraintAdjustment() const
{
    return imp()->data.constraintAdjustment;
}
