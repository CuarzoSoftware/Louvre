#ifndef LKEYBOARDKEYEVENT_H
#define LKEYBOARDKEYEVENT_H

#include <LKeyboardEvent.h>
#include <LKeyboard.h>
#include <LTime.h>

class Louvre::LKeyboardKeyEvent : public LKeyboardEvent
{
public:

    /**
     * @brief Key states.
     *
     * Enum with the possible states of a key.
     */
    enum State : UInt32
    {
        /// The key is not being pressed
        Released = 0,

        /// The key is pressed
        Pressed = 1
    };

    inline LKeyboardKeyEvent(UInt32 keyCode = 0, State state = Pressed, UInt32 serial = LTime::nextSerial(), UInt32 ms = LTime::ms(), UInt64 us = LTime::us(), LInputDevice *device = nullptr) :
        LKeyboardEvent(LEvent::Subtype::Key, serial, ms, us, device),
        m_key(keyCode),
        m_state(state)
    {}

    inline void setKeyCode(UInt32 keyCode)
    {
        m_key = keyCode;
    }

    inline UInt32 keyCode() const
    {
        return m_key;
    }

    inline void setState(State state)
    {
        m_state = state;
    }

    inline State state() const
    {
        return m_state;
    }
protected:
    UInt32 m_key;
    State m_state;
private:
    friend class LInputBackend;
    void notify();
};
#endif // LKEYBOARDKEYEVENT_H
