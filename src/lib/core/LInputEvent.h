#ifndef LINPUTEVENT_H
#define LINPUTEVENT_H

#include <LEvent.h>

class Louvre::LInputEvent : public LEvent
{
public:
    void setDevice(LInputDevice *device);

    inline LInputDevice *device() const
    {
        return m_device;
    }

protected:
    inline LInputEvent(Type type, Subtype subtype, UInt32 serial, UInt32 ms, UInt64 us, LInputDevice *device) :
        LEvent(type, subtype, serial, ms, us)
    {
        setDevice(device);
    }
    LInputDevice *m_device;
};

#endif // LINPUTEVENT_H
