#ifndef LEVENT_H
#define LEVENT_H

#include <LNamespaces.h>

/**
 *  @brief Base event class.
 */
class Louvre::LEvent
{
public:

    /**
     * @brief Defines the type of event.
     */
    enum class Type : UInt8
    {
        Pointer, ///< Pointer event type.
        Keyboard, ///< Keyboard event type.
        Touch ///< Touch event type.
    };

    /**
     * @brief Defines the subtype of event.
     */
    enum class Subtype : UInt8
    {
        Enter, ///< Enter event subtype.
        Leave, ///< Leave event subtype.
        Up, ///< Up event subtype.
        Down, ///< Down event subtype.
        Move, ///< Move event subtype.
        Button, ///< Button event subtype.
        Key, ///< Key event subtype.
        Modifiers, ///< Modifiers event subtype.
        Scroll, ///< Scroll event subtype.
        Frame, ///< Frame event subtype.
        Cancel, ///< Cancel event subtype.
        SwipeBegin, ///< SwipeBegin event subtype.
        SwipeUpdate, ///< SwipeUpdate event subtype.
        SwipeEnd, ///< SwipeEnd event subtype.
        PinchBegin, ///< PinchBegin event subtype.
        PinchUpdate, ///< PinchUpdate event subtype.
        PinchEnd, ///< PinchEnd event subtype.
        HoldBegin, ///< HoldBegin event subtype.
        HoldEnd ///< HoldEnd event subtype.
    };

    /**
     * @brief Destructor.
     */
    inline ~LEvent() noexcept = default ;

    /**
     * @brief Retrieves the type of the event.
     */
    inline Type type() const noexcept
    {
        return m_type;
    }

    /**
     * @brief Retrieves the subtype of the event.
     */
    inline Subtype subtype() const noexcept
    {
        return m_subtype;
    }

    /**
     * @brief Sets the serial of the event.
     */
    inline void setSerial(UInt32 serial) noexcept
    {
        m_serial = serial;
    }

    /**
     * @brief Retrieves the serial of the event.
     */
    inline UInt32 serial() const noexcept
    {
        return m_serial;
    }

    /**
     * @brief Sets the time the event was generated in milliseconds.
     */
    inline void setMs(UInt32 ms) noexcept
    {
        m_ms = ms;
    }

    /**
     * @brief Retrieves the time the event was generated in milliseconds.
     */
    inline UInt32 ms() const noexcept
    {
        return m_ms;
    }

    /**
     * @brief Sets the time the event was generated in microseconds.
     */
    inline void setUs(UInt32 us) noexcept
    {
        m_us = us;
    }

    /**
     * @brief Retrieves the time the event was generated in microseconds.
     */
    inline UInt64 us() const noexcept
    {
        return m_us;
    }

    /**
     * @brief Creates a deep copy of the event.
     *
     * @return A pointer to the copied event.
     *
     * @note The returned event must be deleted when no longer used.
     */
    LEvent *copy() const noexcept;

protected:
    /// @cond OMIT
    LEvent(Type type, Subtype subtype, UInt32 serial, UInt32 ms, UInt64 us) noexcept;
    Type m_type;
    Subtype m_subtype;
    UInt32 m_serial;
    UInt32 m_ms;
    UInt64 m_us;
    /// @endcond
};

#endif // LEVENT_H
