#ifndef GCURSORSHAPEMANAGER_H
#define GCURSORSHAPEMANAGER_H

#include <CZ/Louvre/LResource.h>
#include <CZ/Core/CZWeak.h>

class CZ::Protocols::CursorShape::GCursorShapeManager final : public LResource
{
public:

    /******************** REQUESTS ********************/

    static void destroy(wl_client *client, wl_resource *resource);
    static void get_pointer(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *pointer);
    static void get_tablet_tool_v2(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *tabletTool);

private:
    friend class RCursorShapeDevice;
    LGLOBAL_INTERFACE
    GCursorShapeManager(wl_client *client, Int32 version, UInt32 id);
    ~GCursorShapeManager() noexcept;
};

#endif // GCURSORSHAPEMANAGER_H
