#ifndef LOUTPUTMODE_H
#define LOUTPUTMODE_H

#include <LObject.h>

/*!
 * @brief Output resolution and refresh rate.
 *
 * The LOutputMode class represents a possible configuration in which an output (LOutput) can operate, specifically
 * the refresh rate and resolution.\n
 * Each LOutput has one or more modes which can be accessed from LOutput::modes() and assigned to an output with LOutput::setMode().\n
 * This class does not have virtual methods and therefore it is not necessary to be reimplemented.
 */
class Louvre::LOutputMode : LObject
{
public:

    LOutputMode(const LOutput *output);
    ~LOutputMode();
    LOutputMode(const LOutputMode&) = delete;
    LOutputMode& operator= (const LOutputMode&) = delete;

    /*!
     * @brief Mode output.
     *
     * Output to which the mode belongs.
     */
    const LOutput *output() const;

    /*!
     * @brief Mode resolution.
     *
     * Dimensions of the output when using this mode in buffer coordinates.
     */
    const LSize &sizeB() const;

    /*!
     * @brief Refresh rate.
     *
     * Refresh rate of the mode in mHz.
     */
    UInt32 refreshRate() const;

    /*!
     * @brief Preferred mode.
     *
     * Indicates if the mode is the preferred one by the output.
     */
    bool isPreferred() const;

    LPRIVATE_IMP(LOutputMode)
};

#endif // LOUTPUTMODE_H
