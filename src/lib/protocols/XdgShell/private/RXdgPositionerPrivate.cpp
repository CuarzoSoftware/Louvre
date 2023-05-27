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

    if (width <= 0 || height <= 0)
    {
        wl_resource_post_error(resource, XDG_POSITIONER_ERROR_INVALID_INPUT, "xdg_positioner.set_size requested with non-positive dimensions");
        return;
    }

    RXdgPositioner *rXdgPositioner = (RXdgPositioner*)wl_resource_get_user_data(resource);
    rXdgPositioner->positioner().imp()->data.sizeS.setW(width);
    rXdgPositioner->positioner().imp()->data.sizeS.setH(height);
}

void RXdgPositioner::RXdgPositionerPrivate::set_anchor_rect(wl_client *client, wl_resource *resource, Int32 x, Int32 y, Int32 width, Int32 height)
{
    L_UNUSED(client);

    if (width <= 0 || height <= 0)
    {
        wl_resource_post_error(resource, XDG_POSITIONER_ERROR_INVALID_INPUT, "xdg_positioner.set_anchor_rect requested with non-positive dimensions");
        return;
    }

    RXdgPositioner *rXdgPositioner = (RXdgPositioner*)wl_resource_get_user_data(resource);
    rXdgPositioner->positioner().imp()->data.anchorRectS.setX(x);
    rXdgPositioner->positioner().imp()->data.anchorRectS.setY(y);
    rXdgPositioner->positioner().imp()->data.anchorRectS.setW(width);
    rXdgPositioner->positioner().imp()->data.anchorRectS.setH(height);
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
    rXdgPositioner->positioner().imp()->data.offsetS.setX(x);
    rXdgPositioner->positioner().imp()->data.offsetS.setY(y);
}

#if LOUVRE_XDG_WM_BASE_VERSION >=3
void RXdgPositioner::RXdgPositionerPrivate::set_reactive(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);
    RXdgPositioner *rXdgPositioner = (RXdgPositioner*)wl_resource_get_user_data(resource);
    rXdgPositioner->positioner().imp()->data.isReactive = true;
}

void RXdgPositioner::RXdgPositionerPrivate::set_parent_size(wl_client *client, wl_resource *resource, Int32 parent_width, Int32 parent_height)
{
    L_UNUSED(client);
    RXdgPositioner *rXdgPositioner = (RXdgPositioner*)wl_resource_get_user_data(resource);
    rXdgPositioner->positioner().imp()->data.parentSizeS.setW(parent_width);
    rXdgPositioner->positioner().imp()->data.parentSizeS.setH(parent_height);
}

void RXdgPositioner::RXdgPositionerPrivate::set_parent_configure(wl_client *client, wl_resource *resource, UInt32 serial)
{
    // TODO
    L_UNUSED(client);
    L_UNUSED(resource);
    L_UNUSED(serial);
}
#endif
