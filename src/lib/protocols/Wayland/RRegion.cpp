#include <protocols/Wayland/GCompositor.h>
#include <protocols/Wayland/RRegion.h>

using namespace Louvre::Protocols::Wayland;

static const struct wl_region_interface imp = {.destroy = &RRegion::destroy,
                                               .add = &RRegion::add,
                                               .subtract = &RRegion::subtract};

RRegion::RRegion(GCompositor *compositorRes, UInt32 id) noexcept
    : LResource(compositorRes->client(), &wl_region_interface,
                compositorRes->version(), id, &imp) {}

void RRegion::destroy(wl_client * /*client*/, wl_resource *resource) noexcept {
  wl_resource_destroy(resource);
}

void RRegion::add(wl_client * /*client*/, wl_resource *resource, Int32 x,
                  Int32 y, Int32 width, Int32 height) noexcept {
  if (width > LOUVRE_MAX_SURFACE_SIZE)
    width = LOUVRE_MAX_SURFACE_SIZE;
  else if (width <= 0)
    return;

  if (height > LOUVRE_MAX_SURFACE_SIZE)
    height = LOUVRE_MAX_SURFACE_SIZE;
  else if (height <= 0)
    return;

  auto &regionRes{*static_cast<RRegion *>(wl_resource_get_user_data(resource))};

  pixman_region32_union_rect(&regionRes.m_region.m_region,
                             &regionRes.m_region.m_region, x, y, width, height);
}

void RRegion::subtract(wl_client * /*client*/, wl_resource *resource, Int32 x,
                       Int32 y, Int32 width, Int32 height) noexcept {
  if (width > LOUVRE_MAX_SURFACE_SIZE)
    width = LOUVRE_MAX_SURFACE_SIZE;
  else if (width <= 0)
    return;

  if (height > LOUVRE_MAX_SURFACE_SIZE)
    height = LOUVRE_MAX_SURFACE_SIZE;
  else if (height <= 0)
    return;

  auto &regionRes{*static_cast<RRegion *>(wl_resource_get_user_data(resource))};

  pixman_region32_union_rect(&regionRes.m_subtract.m_region,
                             &regionRes.m_subtract.m_region, x, y, width,
                             height);
}
