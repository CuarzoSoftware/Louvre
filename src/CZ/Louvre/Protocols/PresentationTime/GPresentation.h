#ifndef GPRESENTATION_H
#define GPRESENTATION_H

#include <CZ/Louvre/LResource.h>

class CZ::Protocols::PresentationTime::GPresentation final : public LResource
{
public:

    /******************** REQUESTS ********************/

    static void destroy(wl_client *client, wl_resource *resource) noexcept;
    static void feedback(wl_client *client, wl_resource *resource, wl_resource *surface, UInt32 id) noexcept;

    /******************** EVENTS ********************/

    // Since 1
    void clockId(UInt32 clockId) noexcept;

private:
    LGLOBAL_INTERFACE
    GPresentation(wl_client *client, Int32 version, UInt32 id);
    ~GPresentation() noexcept;
};

#endif // GPRESENTATION_H
