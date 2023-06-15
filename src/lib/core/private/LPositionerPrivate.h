#ifndef LPOSITIONERPRIVATE_H
#define LPOSITIONERPRIVATE_H

#include <LPositioner.h>

using namespace Louvre;

LPRIVATE_CLASS(LPositioner)

    struct PositionerData
    {
        LSize sizeS, sizeC;
        LRect anchorRectS, anchorRectC;
        LPoint offsetS, offsetC;

        UInt32 anchor = Anchor::NoAnchor;
        UInt32 gravity = Gravity::NoGravity;
        UInt32 constraintAdjustment = ConstraintAdjustment::NoAdjustment;

        // Since 3
        bool isReactive = false;
        LSize parentSizeS, parentSizeC;
        UInt32 parentConfigureSerial;
    } data;

    void updateGlobalScale();
};

#endif // LPOSITIONERPRIVATE_H
