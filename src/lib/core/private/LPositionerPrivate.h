#ifndef LPOSITIONERPRIVATE_H
#define LPOSITIONERPRIVATE_H

#include <LPositioner.h>

using namespace Louvre;

LPRIVATE_CLASS(LPositioner)

    struct PositionerData
    {
        LSize size;
        LRect anchorRect;
        LPoint offset;

        UInt32 anchor = Anchor::NoAnchor;
        UInt32 gravity = Gravity::NoGravity;
        UInt32 constraintAdjustment = ConstraintAdjustment::NoAdjustment;

        // Since 3
        bool isReactive = false;
        LSize parentSize;
        UInt32 parentConfigureSerial;
    } data;

    LSize unconstrainedSize;

    void updateGlobalScale();
};

#endif // LPOSITIONERPRIVATE_H
