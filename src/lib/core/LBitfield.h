#ifndef LBITFIELD_H
#define LBITFIELD_H

#include <LNamespaces.h>

/**
 * @brief Utility for managing bitfields
 *
 * The LBitfield class template facilitates the creation and management of bitfields, offering a cleaner API for flag manipulation.
 * Supported data types include integer types, including enums.
 *
 * Example usage:
 *
 * @code
 * enum MyFlags
 * {
 *      FlagA = 1 << 0,
 *      FlagB = 1 << 1,
 *      FlagC = 1 << 2
 * };
 *
 * LBitfield<MyFlags> myFlags;
 *
 * myFlags.add(MyFlags::FlagA | MyFlags::FlagC);
 *
 * if (myFlags.check(MyFlags::FlagC))
 * {
 *      // do something
 * }
 * @endcode
 */
template <class T>
class Louvre::LBitfield
{
    using Flag = typename std::conditional<
        std::is_enum<T>::value,
        typename std::underlying_type<T>::type,
        T
        >::type;

public:

    /**
     * @brief Constructor for LBitfield
     *
     * Initializes the LBitfield with the specified initial flags. If no flags are provided, the bitfield is constructed empty without any flags.
     *
     * @param flags Initial flags to set (default is 0)
     */
    inline LBitfield(Flag flags = 0) : m_flags(flags) {}

    /**
     * @brief Add new flags to the bitfield
     *
     * Sets the specified flags by combining them with the existing flags using the bitwise OR operator '|'.
     *
     * @param flags The flag or combination of flags to be added
     */
    inline void add(Flag flags)
    {
        m_flags |= flags;
    }

    /**
     * @brief Remove flags from the bitfield
     *
     * Clears the specified flags by performing a bitwise AND operation with the complement of the provided flags.
     *
     * @param flags The flag or combination of flags to be removed
     */
    inline void remove(Flag flags)
    {
        m_flags &= ~flags;
    }

    /**
     * @brief Check if at least one flag exists
     *
     * Checks if at least one of the specified flags is set in the bitfield.
     *
     * @param flags The flag or combination of flags to be checked
     * @return `true` if at least one flag is set, otherwise `false`
     */
    inline bool check(Flag flags) const
    {
        return (m_flags & flags) != 0;
    }

    /**
     * @brief Check if all specified flags exist
     *
     * Checks if all of the specified flags are set in the bitfield.
     *
     * @param flags The flag or combination of flags to be checked
     * @return `true` if all specified flags are set, otherwise `false`
     */
    inline bool checkAll(Flag flags) const
    {
        return (m_flags & flags) == flags;
    }

    /**
     * @brief Get the current set of flags
     *
     * Retrieves the current set of flags stored in the bitfield.
     *
     * @return The current set of flags
     */
    inline Flag get() const
    {
        return m_flags;
    }

    /**
     * @brief Set new flags in the bitfield
     *
     * Replaces the current set of flags in the bitfield with the specified flags.
     *
     * @param flags The new flag or combination of flags to be set
     */
    inline void set(Flag flags)
    {
        m_flags = flags;
    }

    /**
     * @brief Set or unset a specific flag in the bitfield
     *
     * Modifies the specified flag in the bitfield. If `enable` is `true`, the flag is set, otherwise, it is removed.
     *
     * @param flag The flag to be set or unset
     * @param enable If `true`, set the flag, if `false`, remove the flag
     */
    inline void setFlag(Flag flag, bool enable)
    {
        if (enable)
            add(flag);
        else
            remove(flag);
    }

private:
    Flag m_flags;
};

#endif // LBITFIELD_H
