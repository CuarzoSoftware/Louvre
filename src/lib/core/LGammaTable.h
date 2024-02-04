#ifndef LGAMMATABLE_H
#define LGAMMATABLE_H

#include <LNamespaces.h>
#include <string.h>

/**
 * @brief Gamma Correction Table for LOutputs.
 *
 * The LGammaTable class manages gamma correction curves for each RGB component, applicable
 * through LOutput::setGamma() method.
 *
 * The curves are stored in a single array of type UInt16. The table size() indicates how many UInt16
 * values each curve contains. For example, if the table has a size() of 256, the array would have
 * 256 * 3 UInt16 values, where the first 256 represent the red curve, the next 256 represent the
 * green curve, and the remaining values represent the blue curve.
 *
 * The table size must match the LOutput::gammaSize() property of the output to which the correction
 * is applied. If not, LOutput::setGamma() will fail. To modify table values, use the fill() auxiliary
 * method or directly modify each curve using red(), green(), and blue(), which return the address of
 * each curve in the array.
 *
 * @see LOuput::gammaSize()
 * @see LOuput::setGamma()
 * @see LOuput::setGammaRequest()
 */
class Louvre::LGammaTable
{
public:

    /**
     * @brief Constructs an LGammaTable with the specified size.
     *
     * @param size The size of the gamma correction table. Defaults to 0.
     */
    inline LGammaTable(UInt32 size = 0)
    {
        setSize(size);
    }

    /**
     * @brief Destructor for LGammaTable.
     *
     * Destroys the gamma correction table.
     */
    inline ~LGammaTable()
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
    inline LGammaTable(const LGammaTable &other)
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
    LGammaTable &operator=(const LGammaTable &other)
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
    inline void setSize(UInt32 size)
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
     * The size represents the number of UInt16 values used to independently represent each RGB curve.
     *
     * @return The size of the gamma correction table.
     */
    inline UInt32 size() const
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
    void fill(Float64 gamma, Float64 brightness, Float64 contrast);

    /**
     * @brief Get a pointer to the beginning of the red curve in the array.
     *
     * @return Pointer to the red curve in the array, or `nullptr` if the table size is 0.
     */
    inline UInt16 *red() const
    {
        return m_table;
    }

    /**
     * @brief Get a pointer to the beginning of the green curve in the array.
     *
     * @return Pointer to the green curve in the array, or `nullptr` if the table size is 0.
     */
    inline UInt16 *green() const
    {
        return (m_size == 0) ? nullptr : m_table + m_size;
    }

    /**
     * @brief Get a pointer to the beginning of the blue curve in the array.
     *
     * @return Pointer to the blue curve in the array, or `nullptr` if the table size is 0.
     */
    inline UInt16 *blue() const
    {
        return (m_size == 0) ? nullptr : m_table + m_size * 2;
    }

private:
    friend class Protocols::GammaControl::RGammaControl;
    friend class LOutput;

    UInt32 m_size {0};
    UInt16 *m_table {nullptr};

    // This is only set for tables created by a client and is not copied across tables
    Protocols::GammaControl::RGammaControl *m_gammaControlResource {nullptr};
};

#endif // LGAMMATABLE_H
