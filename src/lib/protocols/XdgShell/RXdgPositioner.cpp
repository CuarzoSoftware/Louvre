#include <protocols/XdgShell/RXdgPositioner.h>
#include <protocols/XdgShell/GXdgWmBase.h>
#include <private/LPositionerPrivate.h>

using namespace Louvre::Protocols::XdgShell;

static const struct xdg_positioner_interface imp
{
    .destroy = &RXdgPositioner::destroy,
    .set_size = &RXdgPositioner::set_size,
    .set_anchor_rect = &RXdgPositioner::set_anchor_rect,
    .set_anchor = &RXdgPositioner::set_anchor,
    .set_gravity = &RXdgPositioner::set_gravity,
    .set_constraint_adjustment = &RXdgPositioner::set_constraint_adjustment,
    .set_offset = &RXdgPositioner::set_offset,
#if LOUVRE_XDG_WM_BASE_VERSION >= 3
    .set_reactive = &RXdgPositioner::set_reactive,
    .set_parent_size = &RXdgPositioner::set_parent_size,
    .set_parent_configure = &RXdgPositioner::set_parent_configure
#else
    .set_reactive = NULL,
    .set_parent_size = NULL,
    .set_parent_configure = NULL
#endif
};

RXdgPositioner::RXdgPositioner
(
    GXdgWmBase *xdgWmBaseRes,
    UInt32 id
) noexcept
    :LResource
    (
        xdgWmBaseRes->client(),
        &xdg_positioner_interface,
        xdgWmBaseRes->version(),
        id,
        &imp
    )
{}

bool RXdgPositioner::validate()
{
    if (positioner().size().w() <= 0 || positioner().size().h() <= 0)
    {
        wl_resource_post_error(resource(),
                               XDG_POSITIONER_ERROR_INVALID_INPUT,
                               "xdg_positioner.set_size requested with non-positive dimensions");
        return false;
    }

    if (positioner().anchorRect().w() <= 0 || positioner().anchorRect().h() <= 0)
    {
        wl_resource_post_error(resource(),
                               XDG_POSITIONER_ERROR_INVALID_INPUT,
                               "xdg_positioner.set_anchor_rect requested with non-positive dimensions");
        return false;
    }

    if (positioner().gravity() > 8)
    {
        wl_resource_post_error(resource(),
                               XDG_POSITIONER_ERROR_INVALID_INPUT,
                               "xdg_positioner.set_anchor_rect requested with non-positive dimensions");
        return false;
    }

    return true;
}

/******************** REQUESTS ********************/

void RXdgPositioner::destroy(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}

void RXdgPositioner::set_size(wl_client */*client*/, wl_resource *resource, Int32 width, Int32 height) noexcept
{
    auto &data { static_cast<RXdgPositioner*>(wl_resource_get_user_data(resource))->positioner().imp()->data };
    data.size.setW(width);
    data.size.setH(height);
}

void RXdgPositioner::set_anchor_rect(wl_client */*client*/, wl_resource *resource, Int32 x, Int32 y, Int32 width, Int32 height) noexcept
{
    auto &data { static_cast<RXdgPositioner*>(wl_resource_get_user_data(resource))->positioner().imp()->data };
    data.anchorRect.setX(x);
    data.anchorRect.setY(y);
    data.anchorRect.setW(width);
    data.anchorRect.setH(height);
}

void RXdgPositioner::set_anchor(wl_client */*client*/, wl_resource *resource, UInt32 anchor) noexcept
{
    auto &data { static_cast<RXdgPositioner*>(wl_resource_get_user_data(resource))->positioner().imp()->data };
    data.anchor = anchor;
}

void RXdgPositioner::set_gravity(wl_client */*client*/, wl_resource *resource, UInt32 gravity) noexcept
{
    auto &data { static_cast<RXdgPositioner*>(wl_resource_get_user_data(resource))->positioner().imp()->data };
    data.gravity = gravity;
}

void RXdgPositioner::set_constraint_adjustment(wl_client */*client*/, wl_resource *resource, UInt32 constraintAdjustment) noexcept
{
    auto &data { static_cast<RXdgPositioner*>(wl_resource_get_user_data(resource))->positioner().imp()->data };
    data.constraintAdjustment = constraintAdjustment;
}

void RXdgPositioner::set_offset(wl_client */*client*/, wl_resource *resource, Int32 x, Int32 y) noexcept
{
    auto &data { static_cast<RXdgPositioner*>(wl_resource_get_user_data(resource))->positioner().imp()->data };
    data.offset.setX(x);
    data.offset.setY(y);
}
