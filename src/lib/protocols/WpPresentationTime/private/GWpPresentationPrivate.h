#ifndef GWPPRESENTATIONPRIVATE_H
#define GWPPRESENTATIONPRIVATE_H

#include <protocols/WpPresentationTime/GWpPresentation.h>
#include <protocols/WpPresentationTime/presentation-time.h>

using namespace Louvre::Protocols::WpPresentationTime;
using namespace std;

LPRIVATE_CLASS(GWpPresentation)
    static void bind(wl_client *client, void *data, UInt32 version, UInt32 id);
    static void resource_destroy(wl_resource *resource);
    static void destroy(wl_client *client, wl_resource *resource);
    static void feedback(wl_client *client, wl_resource *resource, wl_resource *surface, UInt32 id);

    list<GWpPresentation*>::iterator clientLink;
};

#endif // GWPPRESENTATIONPRIVATE_H
