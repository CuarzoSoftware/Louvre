#include <private/LXWindowRolePrivate.h>

LXWindowRole::LXWindowRole(const void *params) noexcept : LBaseSurfaceRole(FactoryObjectType,
                       ((LXWindowRole::Params*)params)->xWaylandSurfaceRes,
                       ((LXWindowRole::Params*)params)->surface,
                       LSurface::Role::Toplevel)
{

}
