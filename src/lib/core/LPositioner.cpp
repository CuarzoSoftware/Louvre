#include <private/LPositionerPrivate.h>

LPositioner::LPositioner() : LPRIVATE_INIT_UNIQUE(LPositioner) {}
LPositioner::~LPositioner() {}

const LSize &LPositioner::size() const
{
    return imp()->data.size;
}

const LSize &LPositioner::unconstrainedSize() const
{
    return imp()->unconstrainedSize;
}

void LPositioner::setUnconstrainedSize(const LSize &size) const
{
    imp()->unconstrainedSize = size;
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
