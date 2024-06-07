#ifndef LGAMMATABLE_H
#define LGAMMATABLE_H

#include <protocols/GammaControl/RGammaControl.h>
#include <LNamespaces.h>
#include <string.h>
#include <LWeak.h>

/**
 * @brief Gamma correction table for outputs.
 *
 * @anchor lgammatable_detailed
 *
 * The LGammaTable class allows to define the gamma correction curves for each RGB component of an LOutput, applicable
 * through the LOutput::setGamma() method.
 *
 * @note Clients using the Wlr Gamma Control protocol can also request to set the gamma table of an output via LOutput::setGammaRequest().
 *
 * The curves (red, green and blue) are stored in a single array of type @ref UInt16.
 * The table size (size()) indicates how many @ref UInt16 values each curve contains.
 * For example, if size() is 256, the array would have 256 * 3 @ref UInt16 values, where the first 256 represent the red curve, the next 256 represent the
 * green curve, and the remaining values represent the blue curve.
 *
 * The table size must match the LOutput::gammaSize() property of the output to which the correction
 * is applied. If not, LOutput::setGamma() will fail. To modify table values, you can use the fill() auxiliary
 * method or directly modify each curve using red(), green(), and blue(), which return the address of
 * each curve in the array.
 *
 * @see LOutput::gammaSize()
 * @see LOutput::setGamma()
 * @see LOutput::setGammaRequest()
 */
class Louvre::LGammaTable
{
public:

    /**
     * @brief Constructs an LGammaTable with the specified size.
     *
     * @param size The size of the gamma correction table. Defaults to 0.
     */
    LGammaTable(UInt32 size = 0) noexcept
    {
        setSize(size);
    }

    /**
     * @brief Destructor for LGammaTable.
     *
     * Destroys the gamma correction table.
     */
    ~LGammaTable()
    {
        setSize(0);
    }

    /**
     * @brief Copy constructor for LGammaTable.
     *
     * Creates a new LGammaTable by copying the content of another table.
     *
     * @param other The LGammaTable object to be copied.
     */
    LGammaTable(const LGammaTable &other) noexcept
    {
        setSize(other.size());
        if (size() > 0)
            memcpy(m_table, other.m_table, size() * 3 * sizeof(UInt16));
    }

    /**
     * @brief Assignment operator for LGammaTable.
     *
     * Assigns the content of another LGammaTable to this table.
     *
     * @param other The LGammaTable object to be assigned.
     * @return A reference to the modified LGammaTable.
     */
    LGammaTable &operator=(const LGammaTable &other) noexcept
    {
        setSize(other.size());
        if (size() > 0)
            memcpy(m_table, other.m_table, size() * 3 * sizeof(UInt16));
        return *this;
    }

    /**
     * @brief Set the size of the gamma correction table.
     *
     * @note This operation removes previous values.
     *
     * @param size The new size for the gamma correction table.
     */
    void setSize(UInt32 size) noexcept
    {
        if (size != m_size)
        {
            m_size = size;

            if (m_table)
            {
                delete[] m_table;
                m_table = nullptr;
            }

            if (size > 0)
                m_table = new UInt16[size * 3];
        }
    }

    /**
     * @brief Get the size of the gamma correction table.
     *
     * The size represents the number of @ref UInt16 values used to independently represent each RGB curve.
     *
     * @return The size of the gamma correction table.
     */
    UInt32 size() const noexcept
    {
        return m_size;
    }

    /**
     * @brief Fill method for setting table values.
     *
     * This auxiliary method allows setting gamma, brightness, and contrast values for the table.
     *
     * @param gamma The gamma correction value.
     * @param brightness The brightness adjustment value.
     * @param contrast The contrast adjustment value.
     */
    void fill(Float64 gamma, Float64 brightness, Float64 contrast) noexcept;

    /**
     * @brief Get a pointer to the beginning of the red curve in the array.
     *
     * @return Pointer to the red curve in the array, or `nullptr` if the table size is 0.
     */
    UInt16 *red() const noexcept
    {
        return m_table;
    }

    /**
     * @brief Get a pointer to the beginning of the green curve in the array.
     *
     * @return Pointer to the green curve in the array, or `nullptr` if the table size is 0.
     */
    UInt16 *green() const noexcept
    {
        return (m_size == 0) ? nullptr : m_table + m_size;
    }

    /**
     * @brief Get a pointer to the beginning of the blue curve in the array.
     *
     * @return Pointer to the blue curve in the array, or `nullptr` if the table size is 0.
     */
    UInt16 *blue() const noexcept
    {
        return (m_size == 0) ? nullptr : m_table + m_size * 2;
    }

private:
    friend class Protocols::GammaControl::RGammaControl;
    friend class LOutput;

    // This is only set for tables created by a client and is not copied across tables
    LWeak<Protocols::GammaControl::RGammaControl> m_gammaControlResource;
    UInt16 *m_table {nullptr};
    UInt32 m_size {0};
};

#endif // LGAMMATABLE_H
