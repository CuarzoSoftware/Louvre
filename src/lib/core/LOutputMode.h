#ifndef LOUTPUTMODE_H
#define LOUTPUTMODE_H

#include <LObject.h>
#include <memory>

/**
 * @brief Output resolution and refresh rate.
 *
 * The LOutputMode class represents a possible configuration in which an LOutput can operate, specifically
 * the refresh rate and resolution.\n
 * Each LOutput has one or more modes which can be accessed from LOutput::modes() and assigned with LOutput::setMode().\n
 */
class Louvre::LOutputMode : LObject
{
public:
    LOutputMode(LOutput *output) noexcept;
    ~LOutputMode();
    LCLASS_NO_COPY(LOutputMode)

    /**
     * @brief Get the output associated with this mode.
     *
     * This method retrieves the output to which the mode belongs.
     */
    LOutput *output() const;

    /**
     * @brief Get the resolution of the mode.
     *
     * This method returns the dimensions of the output when using this mode, represented in buffer coordinates.
     */
    const LSize &sizeB() const;

    /**
     * @brief Get the refresh rate of the mode.
     *
     * This method returns the refresh rate of the mode in Hertz (Hz) multiplied by 1000.
     *
     * @return The refresh rate of the mode, expressed in Hz * 1000.
     */
    UInt32 refreshRate() const;

    /**
     * @brief Check if this mode is the preferred mode for the output.
     *
     * This method indicates whether the mode is the preferred choice for the output.
     *
     * @return `true` if this mode is preferred and `false` otherwise.
     */
    bool isPreferred() const;

    LPRIVATE_IMP_UNIQUE(LOutputMode)
};

#endif // LOUTPUTMODE_H
