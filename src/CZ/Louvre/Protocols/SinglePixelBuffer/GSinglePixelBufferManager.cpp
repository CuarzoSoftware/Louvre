#include <CZ/Louvre/Protocols/SinglePixelBuffer/single-pixel-buffer-v1.h>
#include <CZ/Louvre/Protocols/SinglePixelBuffer/GSinglePixelBufferManager.h>
#include <CZ/Louvre/Protocols/SinglePixelBuffer/LSinglePixelBuffer.h>
#include <CZ/Louvre/Private/LClientPrivate.h>
#include <LUtils.h>

using namespace Louvre::Protocols::SinglePixelBuffer;
using namespace Louvre;

static const struct wp_single_pixel_buffer_manager_v1_interface imp
{
    .destroy = &GSinglePixelBufferManager::destroy,
    .create_u32_rgba_buffer = &GSinglePixelBufferManager::create_u32_rgba_buffer
};

void GSinglePixelBufferManager::bind(wl_client *client, void */*data*/, UInt32 version, UInt32 id) noexcept
{
    new GSinglePixelBufferManager(client, version, id);
}

Int32 GSinglePixelBufferManager::maxVersion() noexcept
{
    return LOUVRE_SINGLE_PIXEL_BUFFER_MANAGER_VERSION;
}

const wl_interface *GSinglePixelBufferManager::interface() noexcept
{
    return &wp_single_pixel_buffer_manager_v1_interface;
}

GSinglePixelBufferManager::GSinglePixelBufferManager(
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
    this->client()->imp()->singlePixelBufferManagerGlobals.emplace_back(this);
}

GSinglePixelBufferManager::~GSinglePixelBufferManager() noexcept
{
    LVectorRemoveOneUnordered(client()->imp()->singlePixelBufferManagerGlobals, this);
}

/******************** REQUESTS ********************/

void GSinglePixelBufferManager::destroy(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}

void GSinglePixelBufferManager::create_u32_rgba_buffer(wl_client */*client*/, wl_resource *resource, UInt32 id, UInt32 r, UInt32 g, UInt32 b, UInt32 a) noexcept
{
    auto &res { *static_cast<GSinglePixelBufferManager*>(wl_resource_get_user_data(resource)) };
    new LSinglePixelBuffer(res.client(), res.version(), id, {r,g,b,a});
}


