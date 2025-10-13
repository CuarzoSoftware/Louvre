#ifndef CZ_LOUTPUTMODE_H
#define CZ_LOUTPUTMODE_H

#include <CZ/Louvre/LObject.h>

/**
 * @brief Output resolution and refresh rate.
 *
 * The LOutputMode class represents a possible configuration in which an LOutput can operate, specifically
 * the refresh rate and resolution.\n
 * Each LOutput has one or more modes which can be accessed from LOutput::modes() and assigned with LOutput::setMode().
 */
class CZ::LOutputMode : public LObject
{
public:

    virtual LOutput *output() const noexcept = 0;

    /**
     * @brief Gets the resolution of the mode.
     *
     * This method returns the dimensions of the output when using this mode, represented in buffer coordinates.
     */
    virtual SkISize size() const noexcept = 0;

    /**
     * @brief Gets the refresh rate of the mode.
     *
     * This method returns the refresh rate of the mode in Hertz (Hz) multiplied by 1000.
     *
     * @return The refresh rate of the mode, expressed in Hz * 1000.
     */
    virtual UInt32 refreshRate() const noexcept = 0;

    /**
     * @brief Check if this mode is the preferred mode for the output.
     *
     * This method indicates whether the mode is the preferred choice for the output.
     *
     * @return `true` if this mode is preferred and `false` otherwise.
     */
    virtual bool isPreferred() const noexcept = 0;
};

#endif // CZ_LOUTPUTMODE_H
