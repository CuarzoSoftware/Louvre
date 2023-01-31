#include <protocols/XdgShell/xdg-shell.h>
#include <protocols/XdgShell/XdgPositioner.h>

#include <private/LPositionerPrivate.h>

#include <LSurface.h>
#include <LClient.h>
#include <LCompositor.h>
#include <stdio.h>

void Louvre::Extensions::XdgShell::Positioner::destroy_resource(wl_resource *resource)
{
    delete (LPositioner*)wl_resource_get_user_data(resource);
}

void Louvre::Extensions::XdgShell::Positioner::destroy(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);

    wl_resource_destroy(resource);
}

void Louvre::Extensions::XdgShell::Positioner::set_size(wl_client *client, wl_resource *resource, Int32 width, Int32 height)
{
    L_UNUSED(client);

    if (width <= 0 || height <= 0)
    {
        wl_resource_post_error(resource, XDG_POSITIONER_ERROR_INVALID_INPUT,"xdg_positioner.set_size requested with non-positive dimensions");
        return;
    }

    LPositioner *positioner = (LPositioner*)wl_resource_get_user_data(resource);
    positioner->imp()->data.sizeS.setW(width);
    positioner->imp()->data.sizeS.setH(height);
}

void Louvre::Extensions::XdgShell::Positioner::set_anchor_rect(wl_client *client, wl_resource *resource, Int32 x, Int32 y, Int32 width, Int32 height)
{
    L_UNUSED(client);

    if (width <= 0 || height <= 0)
    {
        wl_resource_post_error(resource, XDG_POSITIONER_ERROR_INVALID_INPUT,"xdg_positioner.set_anchor_rect requested with non-positive dimensions");
        return;
    }

    LPositioner *positioner = (LPositioner*)wl_resource_get_user_data(resource);
    positioner->imp()->data.anchorRectS.setX(x);
    positioner->imp()->data.anchorRectS.setY(y);
    positioner->imp()->data.anchorRectS.setW(width);
    positioner->imp()->data.anchorRectS.setH(height);

}

void Louvre::Extensions::XdgShell::Positioner::set_anchor(wl_client *client, wl_resource *resource, UInt32 anchor)
{
    L_UNUSED(client);

    LPositioner *positioner = (LPositioner*)wl_resource_get_user_data(resource);
    positioner->imp()->data.anchor = anchor;
}

void Louvre::Extensions::XdgShell::Positioner::set_gravity(wl_client *client, wl_resource *resource, UInt32 gravity)
{
    L_UNUSED(client);

    LPositioner *positioner = (LPositioner*)wl_resource_get_user_data(resource);
    positioner->imp()->data.gravity = gravity;
}

void Louvre::Extensions::XdgShell::Positioner::set_constraint_adjustment(wl_client *client, wl_resource *resource, UInt32 constraintAdjustment)
{
    L_UNUSED(client);

    LPositioner *positioner = (LPositioner*)wl_resource_get_user_data(resource);
    positioner->imp()->data.constraintAdjustment = constraintAdjustment;
}

void Louvre::Extensions::XdgShell::Positioner::set_offset(wl_client *client, wl_resource *resource, Int32 x, Int32 y)
{
    L_UNUSED(client);

    LPositioner *positioner = (LPositioner*)wl_resource_get_user_data(resource);
    positioner->imp()->data.offsetS.setX(x);
    positioner->imp()->data.offsetS.setY(y);
}

#if LOUVRE_XDG_WM_BASE_VERSION >=3

    void Louvre::Extensions::XdgShell::Positioner::set_reactive(wl_client *client, wl_resource *resource)
    {
        L_UNUSED(client);

        LPositioner *positioner = (LPositioner*)wl_resource_get_user_data(resource);
        positioner->imp()->data.isReactive = true;
    }

    void Louvre::Extensions::XdgShell::Positioner::set_parent_size(wl_client *client, wl_resource *resource, Int32 parentWidth, Int32 parentHeight)
    {
        L_UNUSED(client);

        LPositioner *positioner = (LPositioner*)wl_resource_get_user_data(resource);
        positioner->imp()->data.parentSizeS.setW(parentWidth);
        positioner->imp()->data.parentSizeS.setH(parentHeight);

    }

    void Louvre::Extensions::XdgShell::Positioner::set_parent_configure(wl_client *client, wl_resource *resource, UInt32 serial)
    {
        L_UNUSED(client);
        L_UNUSED(resource);
        L_UNUSED(serial);

        // TODO
    }

#endif
