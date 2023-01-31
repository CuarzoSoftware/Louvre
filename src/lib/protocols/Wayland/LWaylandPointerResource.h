#ifndef LWAYLANDPOINTERRESOURCE_H
#define LWAYLANDPOINTERRESOURCE_H

#include <LResource.h>
#include <LPointer.h>

using namespace Louvre;

class LWaylandSeatGlobal;

class LWaylandPointerResource : public LResource
{
public:
    LWaylandPointerResource(LWaylandSeatGlobal *seatGlobal, Int32 id);
    ~LWaylandPointerResource();

    void sendEnter(LSurface *surface, const LPoint &point);
    void sendLeave(LSurface *surface);
    void sendFrame();
    void sendAxis(double x, double y, UInt32 source);
    void sendAxis(double x, double y);
    void sendMove(const LPoint &localPos);
    void sendButton(LPointer::Button button, LPointer::ButtonState state);

    LWaylandSeatGlobal *seatGlobal() const;

    LPRIVATE_IMP(LWaylandPointerResource);
};
#endif // LWAYLANDPOINTERRESOURCE_H
