#ifndef LRESOURCE_H
#define LRESOURCE_H

#include <LObject.h>
#include <format>
#include <string>

#define LGLOBAL_INTERFACE \
    friend class Louvre::LGlobal;\
    friend class Louvre::LCompositor;\
    static void bind(wl_client *client, void *data, UInt32 version, UInt32 id) noexcept;\
    static Int32 maxVersion() noexcept;\
    static const wl_interface *interface() noexcept;

#define LRES_CAST(type, res) (*static_cast<type*>(wl_resource_get_user_data(res)))

/**
 * @brief Wrapper for native **wl_resource** structs
 *
 * This class serves as a wrapper for the **wl_resource** struct from the original Wayland library.\n
 * Numerous objects expose their LResource objects, allowing you to utilize native Wayland methods if desired.
 *
 * To explore all the globals/resources implemented by Louvre, refer to the ```Louvre/src/lib/protocols``` directory.
 *
 * @note This class is primarily intended for implementing additional protocols beyond those provided by Louvre.
 */
class Louvre::LResource : public LObject
{
public:
    /**
     * @brief Retrieve the wrapped **wl_resource** pointer.
     *
     * This method returns the original **wl_resource** pointer associated with this LResource.
     *
     * @return A pointer to the wrapped **wl_resource**.
     */
    wl_resource *resource() const noexcept { return m_resource; };

    /**
     * @brief Retrieve the client that owns this resource.
     *
     * This method returns the LClient instance to which this resource belongs.
     *
     * @return A pointer to the owning LClient.
     */
    LClient *client() const noexcept { return m_client; }

    /**
     * @brief Retrieve the version of the global interface.
     *
     * This method returns the version of the global interface associated with this resource.
     *
     * @return The version of the global interface.
     */
    Int32 version() const noexcept;

    /**
     * @brief Retrieve the ID of the resource.
     *
     * This method returns the ID of the resource, which is the same as calling **wl_resource_get_id()**.
     *
     * @return The ID of the resource.
     */
    UInt32 id() const noexcept;

    /**
     * @brief Destroy the resource.
     *
     * This method destroys the resource, equivalent to calling **wl_resource_destroy()**.
     */
    void destroy();

    /**
     * @brief A safe wrapper for wl_resource_post_error.
     *
     * This method ensures that `wl_resource_post_error` is not called more than once for the same client.
     *
     * @param code The error code to be sent to the client.
     * @param message The format string for the error message.
     * @param args Additional arguments to format the message.
     *
     * @warning Calling this method **immediately destroys** the client and all its resources. See destroyLater()
     */
    template<typename... Args>
    void postError(UInt32 code, std::format_string<Args...> message, Args&&... args)
    {
        postErrorPrivate(code, std::format(message, std::forward<Args>(args)...));
    }

protected:

    /**
     * @brief Constructor for LResource using **wl_client** and other parameters.
     *
     * This constructor creates an LResource and registers it with the provided parameters.
     *
     * @param client The **wl_client** associated with the resource.
     * @param interface Pointer to the **wl_interface** provided by the protocol.
     * @param version Version of the global interface.
     * @param id ID of the resource; passing 0 automatically generates an increasing ID.
     * @param implementation Pointer to the interface implementation (struct with callback functions to handle client requests).
     */
    LResource(wl_client *client,
              const wl_interface *interface,
              Int32 version,
              UInt32 id,
              const void *implementation) noexcept;

    /**
     * @brief Constructor for LResource using LClient and other parameters.
     *
     * This constructor creates an LResource and registers it with the provided parameters.
     *
     * @param client The LClient associated with the resource.
     * @param interface Pointer to the wl_interface provided by the protocol.
     * @param version Version of the global interface.
     * @param id ID of the resource; passing 0 automatically generates an increasing ID.
     * @param implementation Pointer to the interface implementation (struct with callback functions to handle client requests).
     */
    LResource(LClient *client,
              const wl_interface *interface,
              Int32 version,
              UInt32 id,
              const void *implementation) noexcept;

    LCLASS_NO_COPY(LResource)

    /**
     * @brief Destructor for LResource.
     */
    ~LResource() { notifyDestruction(); };

private:
    void postErrorPrivate(UInt32 code, const std::string &message);
    LClient *m_client;
    wl_resource *m_resource;
};

#endif // LRESOURCE_H
