#include <protocols/RelativePointer/private/RRelativePointerPrivate.h>
#include <protocols/Wayland/private/RPointerPrivate.h>
#include <protocols/RelativePointer/relative-pointer-unstable-v1.h>
#include <LPointerMoveEvent.h>

static struct zwp_relative_pointer_v1_interface zwp_relative_pointer_v1_implementation =
{
    .destroy = &RRelativePointer::RRelativePointerPrivate::destroy
};

RRelativePointer::RRelativePointer(Wayland::RPointer *rPointer, Int32 id, UInt32 version) :
    LResource(
        rPointer->client(),
        &zwp_relative_pointer_v1_interface,
        version,
        id,
        &zwp_relative_pointer_v1_implementation),
    LPRIVATE_INIT_UNIQUE(RRelativePointer)
{
    imp()->rPointer = rPointer;
    rPointer->imp()->relativePointerResources.push_back(this);
}

RRelativePointer::~RRelativePointer()
{
    if (pointerResource())
        LVectorRemoveOneUnordered(pointerResource()->imp()->relativePointerResources, this);
}

RPointer *RRelativePointer::pointerResource() const
{
    return imp()->rPointer;
}

bool RRelativePointer::relativeMotion(const Louvre::LPointerMoveEvent &event)
{
    zwp_relative_pointer_v1_send_relative_motion(
        resource(),
        event.us() >> 32,
        event.us() & 0xffffffff,
        wl_fixed_from_double(event.delta().x()),
        wl_fixed_from_double(event.delta().y()),
        wl_fixed_from_double(event.deltaUnaccelerated().x()),
        wl_fixed_from_double(event.deltaUnaccelerated().y()));
    return true;
}
