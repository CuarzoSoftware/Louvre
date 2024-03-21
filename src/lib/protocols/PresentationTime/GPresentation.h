#ifndef GPRESENTATION_H
#define GPRESENTATION_H

#include <LResource.h>
#include <protocols/PresentationTime/presentation-time.h>

class Louvre::Protocols::PresentationTime::GPresentation final : public LResource
{
public:

    /******************** REQUESTS ********************/

    static void bind(wl_client *client, void *data, UInt32 version, UInt32 id) noexcept;
    static void destroy(wl_client *client, wl_resource *resource) noexcept;
    static void feedback(wl_client *client, wl_resource *resource, wl_resource *surface, UInt32 id) noexcept;

    /******************** EVENTS ********************/

    // Since 1
    void clockId(UInt32 clockId) noexcept;

private:
    GPresentation(wl_client *client, Int32 version, UInt32 id) noexcept;
    ~GPresentation() noexcept;
};

#endif // GPRESENTATION_H
