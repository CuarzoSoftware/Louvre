#ifndef CZ_LSURFACECOMMITEVENT_H
#define CZ_LSURFACECOMMITEVENT_H

#include <CZ/Louvre/Louvre.h>
#include <CZ/Core/Events/CZEvent.h>
#include <CZ/Core/CZBitset.h>

class CZ::LSurfaceCommitEvent : public CZEvent
{
public:
    enum Changes : UInt32
    {
        NoChanges                       = 0U,

        BufferSizeChanged               = 1U << 0,
        BufferScaleChanged              = 1U << 1,
        BufferTransformChanged          = 1U << 2,
        SrcRectChanged                  = 1U << 3,
        SizeChanged                     = 1U << 4,

        DamageRegionChanged             = 1U << 5,
        OpaqueRegionChanged             = 1U << 6,
        InputRegionChanged              = 1U << 7,
        InvisibleRegionChanged          = 1U << 8,

        PointerConstraintModeChanged    = 1U << 9,
        PointerConstraintRegionChanged  = 1U << 10,
        LockedPointerPosHintChanged     = 1U << 11,

        VSyncChanged                    = 1U << 12,
        ContentTypeChanged              = 1U << 13,

        MappingChanged                  = 1U << 14,
    };

    CZ_EVENT_DECLARE_COPY

    LSurfaceCommitEvent(CZBitset<Changes> changes) noexcept :
        CZEvent(Type::LSurfaceCommit),
        changes(changes) {};
    CZBitset<Changes> changes;
};

#endif // CZ_LSURFACECOMMITEVENT_H
