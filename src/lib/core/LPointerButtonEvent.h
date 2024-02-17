#ifndef LPOINTERBUTTONEVENT_H
#define LPOINTERBUTTONEVENT_H

#include <LPointerEvent.h>
#include <linux/input-event-codes.h>
#include <LTime.h>

class Louvre::LPointerButtonEvent : public LPointerEvent
{
public:

    /**
     * @brief Pointer buttons.
     *
     * Enumeration of common pointer buttons.
     *
     * You can find the complete list of pointer button codes in the [`<linux/input-event-codes.h>`](https://github.com/torvalds/linux/blob/master/include/uapi/linux/input-event-codes.h) header.
     */
    enum Button : UInt32
    {
        /// Left button
        Left = BTN_LEFT,

        /// Right button
        Right = BTN_RIGHT,

        /// Middle button
        Middle = BTN_MIDDLE,

        /// Side button
        Side = BTN_SIDE,

        /// Extra button
        Extra = BTN_EXTRA,

        /// Forward button
        Forward = BTN_FORWARD,

        /// Back button
        Back = BTN_BACK,

        /// Task button
        Task = BTN_TASK
    };

    /**
     * @brief Pointer button states.
     *
     * Possible states of a pointer button.
     */
    enum State : UInt32
    {
        /// Button released
        Released = 0,

        /// Button pressed
        Pressed = 1
    };

    inline LPointerButtonEvent(Button button = Left, State state = Pressed, UInt32 serial = LTime::nextSerial(),
                               UInt32 ms = LTime::ms(), UInt64 us = LTime::us(), LInputDevice *device = nullptr) :
        LPointerEvent(LEvent::Subtype::Button, serial, ms, us, device),
        m_button(button),
        m_state(state)
    {}

    inline void setButton(Button button)
    {
        m_button = button;
    }

    inline Button button() const
    {
        return m_button;
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
    Button m_button;
    State m_state;
private:
    friend class LInputBackend;
    void notify();
};

#endif // LPOINTERBUTTONEVENT_H
