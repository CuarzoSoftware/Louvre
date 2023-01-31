#ifndef LOUTPUTMODE_H
#define LOUTPUTMODE_H

#include <LNamespaces.h>

/*!
 * @brief Output resolution and refresh rate.
 *
 * The LOutputMode class represents a possible configuration in which an output (LOutput) can operate, specifically
 * the refresh rate and resolution.\n
 * Each LOutput has one or more modes which can be accessed from LOutput::modes() and assigned to an output with LOutput::setMode().\n
 * This class does not have virtual methods and therefore it is not necessary to be reimplemented.
 */
class Louvre::LOutputMode
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

    class LOutputModePrivate;

    /*!
     * @brief Access to the private API of LOutputMode.
     *
     * Returns an instance of the LOutputModePrivate class (following the ***PImpl Idiom*** pattern) which contains all the private members of LOutputMode.\n
     * Used internally by the library.
     */
    LOutputModePrivate *imp() const;
private:

    LOutputModePrivate *m_imp = nullptr;

};

#endif // LOUTPUTMODE_H
