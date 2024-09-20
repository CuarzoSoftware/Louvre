#ifndef LOUTPUTMODE_H
#define LOUTPUTMODE_H

#include <LObject.h>
#include <LSize.h>
#include <LWeak.h>

/**
 * @brief Output resolution and refresh rate.
 *
 * The LOutputMode class represents a possible configuration in which an LOutput can operate, specifically
 * the refresh rate and resolution.\n
 * Each LOutput has one or more modes which can be accessed from LOutput::modes() and assigned with LOutput::setMode().
 */
class Louvre::LOutputMode final : LObject
{
public:
    LOutputMode(LOutput *output, const LSize &size, UInt32 refreshRate, bool isPreferred, void *data) noexcept;
    LCLASS_NO_COPY(LOutputMode)
    ~LOutputMode() { notifyDestruction(); };

    /**
     * @brief Gets the output associated with this mode.
     *
     * This method retrieves the output to which the mode belongs.
     */
    LOutput *output() const noexcept
    {
        return m_output;
    }

    /**
     * @brief Gets the resolution of the mode.
     *
     * This method returns the dimensions of the output when using this mode, represented in buffer coordinates.
     */
    const LSize &sizeB() const noexcept
    {
        return m_sizeB;
    }

    /**
     * @brief Gets the refresh rate of the mode.
     *
     * This method returns the refresh rate of the mode in Hertz (Hz) multiplied by 1000.
     *
     * @return The refresh rate of the mode, expressed in Hz * 1000.
     */
    UInt32 refreshRate() const noexcept
    {
        return m_refreshRate;
    }

    /**
     * @brief Check if this mode is the preferred mode for the output.
     *
     * This method indicates whether the mode is the preferred choice for the output.
     *
     * @return `true` if this mode is preferred and `false` otherwise.
     */
    bool isPreferred() const noexcept
    {
        return m_isPreferred;
    }

    /**
     * @brief Backend data handle.
     */
    void *data() const noexcept
    {
        return m_data;
    }

private:
    friend class LGraphicBackend;
    LSize m_sizeB;
    UInt32 m_refreshRate;
    LWeak<LOutput> m_output;
    void *m_data { nullptr };
    bool m_isPreferred;
};

#endif // LOUTPUTMODE_H
