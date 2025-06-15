#include <CZ/Louvre/Protocols/XdgShell/xdg-shell.h>
#include <CZ/Louvre/Protocols/XdgShell/RXdgPositioner.h>
#include <CZ/Louvre/Protocols/XdgShell/GXdgWmBase.h>
#include <LPositioner.h>

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
        postError(
            XDG_POSITIONER_ERROR_INVALID_INPUT,
            "xdg_positioner.set_size requested with non-positive dimensions ({}, {})",
            positioner().size().w(), positioner().size().h());
        return false;
    }

    if (positioner().anchorRect().w() <= 0 || positioner().anchorRect().h() <= 0)
    {
        postError(
            XDG_POSITIONER_ERROR_INVALID_INPUT,
            "xdg_positioner.set_anchor_rect requested with non-positive dimensions ({}, {})",
            positioner().anchorRect().w(), positioner().anchorRect().h());
        return false;
    }

    if (positioner().gravity() > 8)
    {
        postError(
            XDG_POSITIONER_ERROR_INVALID_INPUT,
            "Invalid gravity.");
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
    auto &positioner { static_cast<RXdgPositioner*>(wl_resource_get_user_data(resource))->m_positioner };
    positioner.m_size.setW(width);
    positioner.m_size.setH(height);
}

void RXdgPositioner::set_anchor_rect(wl_client */*client*/, wl_resource *resource, Int32 x, Int32 y, Int32 width, Int32 height) noexcept
{
    auto &positioner { static_cast<RXdgPositioner*>(wl_resource_get_user_data(resource))->m_positioner };
    positioner.m_anchorRect.setX(x);
    positioner.m_anchorRect.setY(y);
    positioner.m_anchorRect.setW(width);
    positioner.m_anchorRect.setH(height);
}

void RXdgPositioner::set_anchor(wl_client */*client*/, wl_resource *resource, UInt32 anchor) noexcept
{
    auto &positioner { static_cast<RXdgPositioner*>(wl_resource_get_user_data(resource))->m_positioner };
    positioner.m_anchor = static_cast<LPositioner::Anchor>(anchor);
}

void RXdgPositioner::set_gravity(wl_client */*client*/, wl_resource *resource, UInt32 gravity) noexcept
{
    auto &positioner { static_cast<RXdgPositioner*>(wl_resource_get_user_data(resource))->m_positioner };
    positioner.m_gravity = static_cast<LPositioner::Gravity>(gravity);
}

void RXdgPositioner::set_constraint_adjustment(wl_client */*client*/, wl_resource *resource, UInt32 constraintAdjustment) noexcept
{
    auto &positioner { static_cast<RXdgPositioner*>(wl_resource_get_user_data(resource))->m_positioner };
    positioner.m_constraintAdjustments = static_cast<LPositioner::ConstraintAdjustments>(constraintAdjustment);
}

void RXdgPositioner::set_offset(wl_client */*client*/, wl_resource *resource, Int32 x, Int32 y) noexcept
{
    auto &positioner { static_cast<RXdgPositioner*>(wl_resource_get_user_data(resource))->m_positioner };
    positioner.m_offset.setX(x);
    positioner.m_offset.setY(y);
}

#if LOUVRE_XDG_WM_BASE_VERSION >= 3

void RXdgPositioner::set_reactive(wl_client */*client*/, wl_resource *resource)
{
    auto &positioner { static_cast<RXdgPositioner*>(wl_resource_get_user_data(resource))->m_positioner };
    positioner.m_reactive = true;
}

void RXdgPositioner::set_parent_size(wl_client */*client*/, wl_resource *resource, Int32 parent_width, Int32 parent_height)
{
    auto &positioner { static_cast<RXdgPositioner*>(wl_resource_get_user_data(resource))->m_positioner };
    positioner.m_hasParentSize = true;

    if (parent_width < 0)
        positioner.m_parentSize.setW(0);
    else
        positioner.m_parentSize.setW(parent_width);

    if (parent_height < 0)
        positioner.m_parentSize.setH(0);
    else
        positioner.m_parentSize.setH(parent_height);
}

void RXdgPositioner::set_parent_configure(wl_client */*client*/, wl_resource *resource, UInt32 serial)
{
    auto &positioner { static_cast<RXdgPositioner*>(wl_resource_get_user_data(resource))->m_positioner };
    positioner.m_hasParentConfigureSerial = true;
    positioner.m_parentConfigureSerial = serial;
}

#endif
