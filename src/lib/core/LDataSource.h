#ifndef LDATASOURCE_H
#define LDATASOURCE_H

#include <LObject.h>
#include <bits/types/FILE.h>

/**
 * @brief Data source exchanged between clients
 *
 * @note This class is primarily intended for internal use by Louvre and may not be directly useful to you.
 *
 * The LDataSource class represents the [wl_data_source](https://wayland.app/protocols/wayland#wl_data_source) interface of the Wayland protocol.
 * It is created by clients to define information that can be shared with other clients through the clipboard or in drag & drop sessions.\n
 *
 * @see LDataDevice
 * @see LDataOffer
 */
class Louvre::LDataSource : LObject
{
public:   

    /**
     * @brief Variant of a data source.
     *
     * Data of the source for a specific mime type.
     */
    struct LSource
    {
        /// @brief File descriptor for a specific mime type
        FILE *tmp;

        /// @brief Mime type
        char *mimeType;
    };

    /// @cond OMIT
    LDataSource(const LDataSource&) = delete;
    LDataSource& operator= (const LDataSource&) = delete;
    /// @endcond

    /**
     * @brief Client owner of the data source.
     */
    LClient *client() const;

    /**
     * @brief List of data source file descriptors.
     *
     * List of data source file descriptors for specific mime types.
     */
    const std::list<LSource> &sources() const;

    // Since 3

    /**
     * @brief Actions available for drag & drop sessions.
     *
     * Flags indicating the actions available for a drag & drop session.\n
     * The possible flags are defined in LDNDManager::Action.
     */
    UInt32 dndActions() const;

    /**
     * @brief Wayland resource of the data source.
     *
     * @returns the resource generated by the [wl_data_source](https://wayland.app/protocols/wayland#wl_data_source) interface of the Wayland protocol.
     */
    Protocols::Wayland::RDataSource *dataSourceResource() const;

    LPRIVATE_IMP_UNIQUE(LDataSource)

    /// @cond OMIT
    friend class Protocols::Wayland::RDataSource;
    friend class Protocols::Wayland::RDataDevice;
    LDataSource(Protocols::Wayland::RDataSource *dataSourceResource);
    ~LDataSource();
    /// @endcond
};

#endif // LDATASOURCE_H
