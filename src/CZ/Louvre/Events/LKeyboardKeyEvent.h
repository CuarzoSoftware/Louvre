#ifndef LKEYBOARDKEYEVENT_H
#define LKEYBOARDKEYEVENT_H

#include <CZ/Louvre/Events/LKeyboardEvent.h>
#include <CZ/Louvre/LTime.h>

/**
 * @brief Keyboard key event.
 */
class Louvre::LKeyboardKeyEvent final : public LKeyboardEvent
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

    /**
     * @brief Constructor for LKeyboardKeyEvent.
     *
     * @param keyCode The raw key code.
     * @param state The state of the key (Pressed or Released).
     * @param serial The serial number of the event.
     * @param ms The millisecond timestamp of the event.
     * @param us The microsecond timestamp of the event.
     * @param device The input device that originated the event.
     */
    LKeyboardKeyEvent(UInt32 keyCode = 0, State state = Pressed, UInt32 serial = LTime::nextSerial(),
                      UInt32 ms = LTime::ms(), UInt64 us = LTime::us(), LInputDevice *device = nullptr) noexcept :
        LKeyboardEvent(LEvent::Subtype::Key, serial, ms, us, device),
        m_key(keyCode),
        m_state(state)
    {}

    /**
     * @brief Sets the raw key code.
     */
    void setKeyCode(UInt32 keyCode) noexcept
    {
        m_key = keyCode;
    }

    /**
     * @brief Gets the raw key code.
     */
    UInt32 keyCode() const noexcept
    {
        return m_key;
    }

    /**
     * @brief Sets the state of the key.
     */
    void setState(State state) noexcept
    {
        m_state = state;
    }

    /**
     * @brief Gets the state of the key.
     */
    State state() const noexcept
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
