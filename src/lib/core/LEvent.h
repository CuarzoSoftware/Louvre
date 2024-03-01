#ifndef LEVENT_H
#define LEVENT_H

#include <LObject.h>

// TODO: add doc
class Louvre::LEvent
{
public:

    enum class Type : UInt8
    {
        Pointer,
        Keyboard,
        Touch
    };

    enum class Subtype : UInt8
    {
        Enter,
        Leave,
        Up,
        Down,
        Move,
        Button,
        Key,
        Modifiers,
        Scroll,
        Frame,
        Cancel,
        SwipeBegin,
        SwipeUpdate,
        SwipeEnd,
        PinchBegin,
        PinchUpdate,
        PinchEnd,
        HoldBegin,
        HoldEnd
    };

    virtual ~LEvent() = default;

    inline Type type() const
    {
        return m_type;
    }

    inline Subtype subtype() const
    {
        return m_subtype;
    }

    inline void setSerial(UInt32 serial)
    {
        m_serial = serial;
    }

    inline UInt32 serial() const
    {
        return m_serial;
    }

    inline void setMs(UInt32 ms)
    {
        m_ms = ms;
    }

    inline UInt32 ms() const
    {
        return m_ms;
    }

    inline void setUs(UInt32 us)
    {
        m_us = us;
    }

    inline UInt64 us() const
    {
        return m_us;
    }

    inline LCompositor *compositor() const
    {
        return LCompositor::compositor();
    }

    inline LSeat *seat() const
    {
        return compositor()->seat();
    }

    LEvent *copy() const;

protected:
    LEvent(Type type, Subtype subtype, UInt32 serial, UInt32 ms, UInt64 us);
    Type m_type;
    Subtype m_subtype;
    UInt32 m_serial;
    UInt32 m_ms;
    UInt64 m_us;
};

#endif // LEVENT_H
