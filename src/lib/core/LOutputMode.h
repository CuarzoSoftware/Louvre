#ifndef LOUTPUTMODE_H
#define LOUTPUTMODE_H

#include <LObject.h>

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
    /// @cond OMIT
    LOutputMode(const LOutput *output);
    ~LOutputMode();
    LOutputMode(const LOutputMode&) = delete;
    LOutputMode& operator= (const LOutputMode&) = delete;
    /// @endcond

    /**
     * @brief Mode output.
     *
     * Output to which the mode belongs.
     */
    const LOutput *output() const;

    /**
     * @brief Mode resolution.
     *
     * Dimensions of the output when using this mode in buffer coordinates.
     */
    const LSize &sizeB() const;

    /**
     * @brief Refresh rate.
     *
     * Refresh rate of the mode in mHz.
     */
    UInt32 refreshRate() const;

    /**
     * @brief Preferred mode.
     *
     * Indicates if the mode is the preferred one by the output.
     */
    bool isPreferred() const;

    LPRIVATE_IMP(LOutputMode)
};

#endif // LOUTPUTMODE_H
