#ifndef CZ_LSURFACEUNLOCKEVENT_H
#define CZ_LSURFACEUNLOCKEVENT_H

#include <CZ/Louvre/Louvre.h>
#include <CZ/Core/Events/CZEvent.h>
#include <CZ/Core/CZWeak.h>

class CZ::LSurfaceUnlockEvent : public CZEvent
{
public:
    LSurfaceUnlockEvent(LSurface *surface, UInt32 commitId) noexcept :
        CZEvent(Type::LSurfaceUnlock),
        surface(surface), commitId(commitId) {}

    CZ_EVENT_DECLARE_COPY

    CZWeak<LSurface> surface;
    UInt32 commitId;
};

#endif // CZ_LSURFACEUNLOCKEVENT_H
