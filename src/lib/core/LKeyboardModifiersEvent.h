#ifndef LKEYBOARDMODIFIERSEVENT_H
#define LKEYBOARDMODIFIERSEVENT_H

#include <LKeyboardEvent.h>
#include <LTime.h>

class Louvre::LKeyboardModifiersEvent : public LKeyboardEvent
{
public:

    /**
     * @brief Keyboard modifiers.
     *
     * Stores the status of keyboard modifiers (Ctrl, Shift, Alt, etc).
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

    inline LKeyboardModifiersEvent(const Modifiers &modifiers = {0, 0, 0, 0},
                                   UInt32 serial = LTime::nextSerial(), UInt32 ms = LTime::ms(),
                                   UInt64 us = LTime::us(), LInputDevice *device = nullptr) :
        LKeyboardEvent(LEvent::Subtype::Modifiers, serial, ms, us, device),
        m_modifiers(modifiers)
    {}

    inline void setModifiers(const Modifiers &modifiers)
    {
        m_modifiers = modifiers;
    }

    inline const Modifiers &modifiers() const
    {
        return m_modifiers;
    }

protected:
    Modifiers m_modifiers;
};

#endif // LKEYBOARDMODIFIERSEVENT_H
