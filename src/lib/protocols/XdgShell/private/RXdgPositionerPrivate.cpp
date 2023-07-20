#include <protocols/XdgShell/private/RXdgPositionerPrivate.h>
#include <private/LPositionerPrivate.h>
#include <protocols/XdgShell/xdg-shell.h>

void RXdgPositioner::RXdgPositionerPrivate::destroy_resource(wl_resource *resource)
{
    RXdgPositioner *rXdgPositioner = (RXdgPositioner*)wl_resource_get_user_data(resource);
    delete rXdgPositioner;
}

void RXdgPositioner::RXdgPositionerPrivate::destroy(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);
    wl_resource_destroy(resource);
}

void RXdgPositioner::RXdgPositionerPrivate::set_size(wl_client *client, wl_resource *resource, Int32 width, Int32 height)
{
    L_UNUSED(client);
    RXdgPositioner *rXdgPositioner = (RXdgPositioner*)wl_resource_get_user_data(resource);
    rXdgPositioner->positioner().imp()->data.size.setW(width);
    rXdgPositioner->positioner().imp()->data.size.setH(height);
}

void RXdgPositioner::RXdgPositionerPrivate::set_anchor_rect(wl_client *client, wl_resource *resource, Int32 x, Int32 y, Int32 width, Int32 height)
{
    L_UNUSED(client);
    RXdgPositioner *rXdgPositioner = (RXdgPositioner*)wl_resource_get_user_data(resource);
    rXdgPositioner->positioner().imp()->data.anchorRect.setX(x);
    rXdgPositioner->positioner().imp()->data.anchorRect.setY(y);
    rXdgPositioner->positioner().imp()->data.anchorRect.setW(width);
    rXdgPositioner->positioner().imp()->data.anchorRect.setH(height);
}

void RXdgPositioner::RXdgPositionerPrivate::set_anchor(wl_client *client, wl_resource *resource, UInt32 anchor)
{
    L_UNUSED(client);
    RXdgPositioner *rXdgPositioner = (RXdgPositioner*)wl_resource_get_user_data(resource);
    rXdgPositioner->positioner().imp()->data.anchor = anchor;
}

void RXdgPositioner::RXdgPositionerPrivate::set_gravity(wl_client *client, wl_resource *resource, UInt32 gravity)
{
    L_UNUSED(client);
    RXdgPositioner *rXdgPositioner = (RXdgPositioner*)wl_resource_get_user_data(resource);
    rXdgPositioner->positioner().imp()->data.gravity = gravity;
}

void RXdgPositioner::RXdgPositionerPrivate::set_constraint_adjustment(wl_client *client, wl_resource *resource, UInt32 constraintAdjustment)
{
    L_UNUSED(client);
    RXdgPositioner *rXdgPositioner = (RXdgPositioner*)wl_resource_get_user_data(resource);
    rXdgPositioner->positioner().imp()->data.constraintAdjustment = constraintAdjustment;
}

void RXdgPositioner::RXdgPositionerPrivate::set_offset(wl_client *client, wl_resource *resource, Int32 x, Int32 y)
{
    L_UNUSED(client);
    RXdgPositioner *rXdgPositioner = (RXdgPositioner*)wl_resource_get_user_data(resource);
    rXdgPositioner->positioner().imp()->data.offset.setX(x);
    rXdgPositioner->positioner().imp()->data.offset.setY(y);
}
