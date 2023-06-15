#include <private/LPositionerPrivate.h>

LPositioner::LPositioner()
{
    m_imp = new LPositionerPrivate();
}

LPositioner::~LPositioner()
{
    delete m_imp;
}

const LSize &LPositioner::sizeS() const
{
    return imp()->data.sizeS;
}

const LSize &LPositioner::sizeC() const
{
    return imp()->data.sizeC;
}

const LRect &LPositioner::anchorRectS() const
{
    return imp()->data.anchorRectS;
}

const LRect &LPositioner::anchorRectC() const
{
    return imp()->data.anchorRectC;
}

const LPoint &LPositioner::offsetS() const
{
    return imp()->data.offsetS;
}

const LPoint &LPositioner::offsetC() const
{
    return imp()->data.offsetC;
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

// Since 3

bool LPositioner::isReactive() const
{
    return imp()->data.isReactive;
}

const LSize &LPositioner::parentSizeS() const
{
    return imp()->data.parentSizeS;
}

const LSize &LPositioner::parentSizeC() const
{
    return imp()->data.parentSizeC;
}

UInt32 LPositioner::parentConfigureSerial() const
{
    return imp()->data.parentConfigureSerial;
}
