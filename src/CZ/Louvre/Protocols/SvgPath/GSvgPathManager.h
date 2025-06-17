#ifndef GSVGPATHMANAGER_H
#define GSVGPATHMANAGER_H

#include <CZ/Louvre/LResource.h>

class Louvre::Protocols::SvgPath::GSvgPathManager final : public LResource
{
public:
    static void destroy(wl_client *client, wl_resource *resource);
    static void get_svg_path(wl_client *client, wl_resource *resource, UInt32 id);

private:
    LGLOBAL_INTERFACE
    GSvgPathManager(wl_client *client, Int32 version, UInt32 id) noexcept;
    ~GSvgPathManager() noexcept;
};

#endif // GSVGPATHMANAGER_H
