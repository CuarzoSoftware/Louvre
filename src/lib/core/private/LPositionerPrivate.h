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

        Anchor anchor = Anchor::NoAnchor;
        Gravity gravity = Gravity::NoGravity;
        ConstraintAdjustments constraintAdjustments = ConstraintAdjustments::NoAdjustment;

        // Since 3
        bool isReactive = false;
        LSize parentSize;
        UInt32 parentConfigureSerial;
    } data;

    LSize unconstrainedSize;

    void updateGlobalScale();
};

#endif // LPOSITIONERPRIVATE_H
