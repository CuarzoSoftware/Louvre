#include <protocols/ScreenCopy/wlr-screencopy-unstable-v1.h>
#include <protocols/ScreenCopy/GScreenCopyManager.h>
#include <protocols/ScreenCopy/RScreenCopyFrame.h>
#include <protocols/Wayland/GOutput.h>
#include <private/LCompositorPrivate.h>
#include <private/LClientPrivate.h>
#include <LOutput.h>
#include <LUtils.h>

using namespace Louvre::Protocols::ScreenCopy;

static const struct zwlr_screencopy_manager_v1_interface imp
{
    .capture_output = &GScreenCopyManager::capture_output,
    .capture_output_region = &GScreenCopyManager::capture_output_region,
    .destroy = &GScreenCopyManager::destroy
};

void GScreenCopyManager::bind(wl_client *client, void */*data*/, UInt32 version, UInt32 id) noexcept
{
    new GScreenCopyManager(client, version, id);
}

Int32 GScreenCopyManager::maxVersion() noexcept
{
    return LOUVRE_SCREEN_COPY_MANAGER_VERSION;
}

const wl_interface *GScreenCopyManager::interface() noexcept
{
    return &zwlr_screencopy_manager_v1_interface;
}

GScreenCopyManager::GScreenCopyManager
    (
        wl_client *client,
        Int32 version,
        UInt32 id
        ) noexcept
    :LResource
    (
        client,
        interface(),
        version,
        id,
        &imp
    )
{
    this->client()->imp()->screenCopyManagerGlobals.emplace_back(this);
    compositor()->imp()->screenshotManagers++;
}

GScreenCopyManager::~GScreenCopyManager() noexcept
{
    LVectorRemoveOneUnordered(client()->imp()->screenCopyManagerGlobals, this);
    compositor()->imp()->screenshotManagers--;
}

/******************** REQUESTS ********************/

void GScreenCopyManager::destroy(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}

void GScreenCopyManager::capture_output(wl_client */*client*/, wl_resource *resource, UInt32 id, Int32 overlayCursor, wl_resource *output) noexcept
{
    auto &res { *static_cast<GScreenCopyManager*>(wl_resource_get_user_data(resource)) };
    auto *outputRes { static_cast<Wayland::GOutput*>(wl_resource_get_user_data(output)) };

    new RScreenCopyFrame(&res,
                         outputRes->output(),
                         overlayCursor == 1,
                         LRect(LPoint(), outputRes->output() ? outputRes->output()->size() : LSize(1)),
                         id,
                         res.version());
}

void GScreenCopyManager::capture_output_region(wl_client */*client*/, wl_resource *resource, UInt32 id, Int32 overlayCursor, wl_resource *output, Int32 x, Int32 y, Int32 width, Int32 height) noexcept
{
    auto &res { *static_cast<GScreenCopyManager*>(wl_resource_get_user_data(resource)) };
    auto *outputRes { static_cast<Wayland::GOutput*>(wl_resource_get_user_data(output)) };

    new RScreenCopyFrame(&res,
                         outputRes->output(),
                         overlayCursor == 1,
                         LRect(x, y, width, height),
                         id,
                         res.version());
}
