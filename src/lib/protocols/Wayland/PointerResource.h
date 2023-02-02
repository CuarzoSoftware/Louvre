#ifndef POINTERRESOURCE_H
#define POINTERRESOURCE_H

#include <LResource.h>
#include <LPointer.h>

class Louvre::Protocols::Wayland::PointerResource : public LResource
{
public:
    PointerResource(SeatGlobal *seatGlobal, Int32 id);
    ~PointerResource();

    struct LastEventSerials
    {
        UInt32 leave = 0;
        UInt32 enter = 0;
        UInt32 button = 0;
    };

    void sendEnter(LSurface *surface, const LPoint &point);
    void sendLeave(LSurface *surface);
    void sendFrame();
    void sendAxis(double x, double y, UInt32 source);
    void sendAxis(double x, double y);
    void sendMove(const LPoint &localPos);
    void sendButton(LPointer::Button button, LPointer::ButtonState state);

    SeatGlobal *seatGlobal() const;
    const LastEventSerials &serials() const;

    LPRIVATE_IMP(PointerResource);
};
#endif // POINTERRESOURCE_H
