#ifndef LKEYBOARDMODIFIERSEVENT_H
#define LKEYBOARDMODIFIERSEVENT_H

#include <LKeyboardEvent.h>
#include <LTime.h>

/**
 * @brief Keyboard modifiers event.
 *
 * Keyboard modifiers events are automatically sent to client surfaces when they acquire keyboard focus.
 */
class Louvre::LKeyboardModifiersEvent final : public LKeyboardEvent
{
public:

    /**
     * @brief Keyboard modifiers.
     */
    struct Modifiers
    {
        /// Active modifiers when physically pressed
        UInt32 depressed = 0;

        /// Hooked modifiers that will be disabled after a non-modifier key is pressed
        UInt32 latched = 0;

        /// Active modifiers until they are pressed again (e.g. the Shift key)
        UInt32 locked = 0;

        /// Group the above states (use this value if the source of a modifier change is not of your interest)
        UInt32 group = 0;
    };

    /**
     * @brief Constructor for LKeyboardModifiersEvent.
     *
     * @param modifiers The keyboard modifiers to be set.
     * @param serial The serial number of the event.
     * @param ms The millisecond timestamp of the event.
     * @param us The microsecond timestamp of the event.
     * @param device The input device that originated the event.
     */
    LKeyboardModifiersEvent(const Modifiers &modifiers = {0, 0, 0, 0},
                                   UInt32 serial = LTime::nextSerial(), UInt32 ms = LTime::ms(),
                                   UInt64 us = LTime::us(), LInputDevice *device = nullptr) noexcept:
        LKeyboardEvent(LEvent::Subtype::Modifiers, serial, ms, us, device),
        m_modifiers(modifiers)
    {}

    /**
     * @brief Sets the keyboard modifiers for this event.
     */
    void setModifiers(const Modifiers &modifiers) noexcept
    {
        m_modifiers = modifiers;
    }

    /**
     * @brief Gets the keyboard modifiers for this event.
     */
    const Modifiers &modifiers() const noexcept
    {
        return m_modifiers;
    }

protected:
    Modifiers m_modifiers;
};

#endif // LKEYBOARDMODIFIERSEVENT_H
