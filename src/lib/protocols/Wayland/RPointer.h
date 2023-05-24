#ifndef POINTERRESOURCE_H
#define POINTERRESOURCE_H

#include <LResource.h>
#include <LPointer.h>

class Louvre::Protocols::Wayland::RPointer : public LResource
{
public:
    RPointer(GSeat *seatGlobal, Int32 id);
    ~RPointer();

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

    GSeat *seatGlobal() const;
    const LastEventSerials &serials() const;

    LPRIVATE_IMP(RPointer);
};
#endif // POINTERRESOURCE_H
