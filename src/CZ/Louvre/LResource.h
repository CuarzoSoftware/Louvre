#ifndef LRESOURCE_H
#define LRESOURCE_H

#include <CZ/Louvre/LObject.h>
#include <format>
#include <list>
#include <string>

#define LGLOBAL_INTERFACE \
    friend class CZ::LGlobal;\
    friend class CZ::LCompositor;\
    static void Bind(wl_client *client, void *data, UInt32 version, UInt32 id) noexcept;\
    static Int32 MaxVersion() noexcept;\
    static const wl_interface *Interface() noexcept;\
    static bool Probe(CZWeak<LGlobal> **slot) noexcept;

#define LRES_CAST(type, res) (*static_cast<type*>(wl_resource_get_user_data(res)))

#define LGLOBAL_INTERFACE_IMP(GlobalName, GlobalVersion, GlobalIface) \
void GlobalName::Bind(wl_client *client, void* /*data*/, UInt32 version, UInt32 id) noexcept { \
    try { \
        new GlobalName(client, version, id); \
    } catch (const std::bad_alloc &e) { \
        wl_client_post_no_memory(client); \
        LLog(CZError, CZLN, "{}", e.what()); \
    } \
} \
\
Int32 GlobalName::MaxVersion() noexcept \
{ \
    return GlobalVersion; \
} \
\
const wl_interface *GlobalName::Interface() noexcept \
{ \
    return &GlobalIface; \
}

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
class CZ::LResource : public LObject
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

    /**
     * @brief Destructor for LResource.
     */
    ~LResource() noexcept;

private:
    void postErrorPrivate(UInt32 code, const std::string &message);
    LClient *m_client;
    wl_resource *m_resource;
    std::list<LResource*>::iterator m_clientLink;
};

#endif // LRESOURCE_H
