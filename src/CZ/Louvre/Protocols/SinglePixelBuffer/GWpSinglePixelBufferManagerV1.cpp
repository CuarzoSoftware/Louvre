#include <CZ/Louvre/Protocols/SinglePixelBuffer/single-pixel-buffer-v1.h>
#include <CZ/Louvre/Protocols/SinglePixelBuffer/GWpSinglePixelBufferManagerV1.h>
#include <CZ/Louvre/Protocols/SinglePixelBuffer/LSinglePixelBuffer.h>
#include <CZ/Louvre/Private/LClientPrivate.h>
#include <CZ/Louvre/LCompositor.h>
#include <CZ/Louvre/LLog.h>
#include <CZ/Core/Utils/CZVectorUtils.h>
#include <CZ/Core/CZBitset.h>
#include <CZ/Ream/RImage.h>
#include <CZ/Ream/RCore.h>
#include <CZ/Ream/RDevice.h>

using namespace CZ::Protocols::SinglePixelBuffer;
using namespace CZ;

static const struct wp_single_pixel_buffer_manager_v1_interface imp
{
    .destroy = &GWpSinglePixelBufferManagerV1::destroy,
    .create_u32_rgba_buffer = &GWpSinglePixelBufferManagerV1::create_u32_rgba_buffer
};

LGLOBAL_INTERFACE_IMP(GWpSinglePixelBufferManagerV1, LOUVRE_SINGLE_PIXEL_BUFFER_MANAGER_VERSION, wp_single_pixel_buffer_manager_v1_interface)

bool GWpSinglePixelBufferManagerV1::Probe(CZWeak<LGlobal> **slot) noexcept
{
    if (compositor()->wellKnownGlobals.SinglePixelBufferManager)
    {
        LLog(CZError, CZLN, "Failed to create {} global (already created)", Interface()->name);
        return false;
    }

    *slot = &compositor()->wellKnownGlobals.SinglePixelBufferManager;
    return true;
}

GWpSinglePixelBufferManagerV1::GWpSinglePixelBufferManagerV1(
    wl_client *client,
    Int32 version,
    UInt32 id
    )
    :LResource
    (
        client,
        Interface(),
        version,
        id,
        &imp
    )
{
    this->client()->imp()->singlePixelBufferManagerGlobals.emplace_back(this);
}

GWpSinglePixelBufferManagerV1::~GWpSinglePixelBufferManagerV1() noexcept
{
    CZVectorUtils::RemoveOneUnordered(client()->imp()->singlePixelBufferManagerGlobals, this);
}

/******************** REQUESTS ********************/

void GWpSinglePixelBufferManagerV1::destroy(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}

static constexpr UInt32 NormU32ToU8(UInt32 value)
{
    return static_cast<UInt32>((static_cast<UInt64>(value) * 255) / std::numeric_limits<UInt32>::max());
}

static constexpr UInt32 PackARGB8888(UInt32 r, UInt32 g, UInt32 b, UInt32 a)
{
    return (NormU32ToU8(a) << 24) |
           (NormU32ToU8(r) << 16) |
           (NormU32ToU8(g) << 8)  |
           (NormU32ToU8(b) << 0);
}

static constexpr UInt32 PackABGR8888(UInt32 r, UInt32 g, UInt32 b, UInt32 a)
{
    return (NormU32ToU8(a) << 24) |
           (NormU32ToU8(b) << 16) |
           (NormU32ToU8(g) << 8)  |
           (NormU32ToU8(r) << 0);
}

void GWpSinglePixelBufferManagerV1::create_u32_rgba_buffer(wl_client */*client*/, wl_resource *resource, UInt32 id, UInt32 r, UInt32 g, UInt32 b, UInt32 a) noexcept
{
    auto &res { *static_cast<GWpSinglePixelBufferManagerV1*>(wl_resource_get_user_data(resource)) };
    auto ream { RCore::Get() };
    std::shared_ptr<RImage> image;

    RImageConstraints cons {};
    cons.allocator = ream->mainDevice();
    cons.caps[cons.allocator] = RImageCap_Src;

    UInt32 pixel;
    RPixelBufferInfo info {};
    info.pixels = (UInt8*)&pixel;
    info.size = {1, 1};
    info.stride = 4;

    // If opaque
    if (a == std::numeric_limits<UInt32>::max())
    {
        info.format = DRM_FORMAT_XRGB8888;
        auto fmt { ream->mainDevice()->textureFormats().formats().find(info.format) };
        if (fmt != ream->mainDevice()->textureFormats().formats().end())
        {
            pixel = PackARGB8888(r, g, b, a);
            image = RImage::MakeFromPixels(info, *fmt, &cons);
        }

        if (!image)
        {
            info.format = DRM_FORMAT_XBGR8888;
            auto fmt { ream->mainDevice()->textureFormats().formats().find(info.format) };
            if (fmt != ream->mainDevice()->textureFormats().formats().end())
            {
                pixel = PackABGR8888(r, g, b, a);
                image = RImage::MakeFromPixels(info, *fmt, &cons);
            }
        }
    }

    if (!image)
    {
        info.format = DRM_FORMAT_ARGB8888;
        auto fmt { ream->mainDevice()->textureFormats().formats().find(info.format) };
        if (fmt != ream->mainDevice()->textureFormats().formats().end())
        {
            pixel = PackARGB8888(r, g, b, a);
            image = RImage::MakeFromPixels(info, *fmt, &cons);
        }
    }

    if (!image)
    {
        info.format = DRM_FORMAT_ABGR8888;
        auto fmt { ream->mainDevice()->textureFormats().formats().find(info.format) };
        if (fmt != ream->mainDevice()->textureFormats().formats().end())
        {
            pixel = PackABGR8888(r, g, b, a);
            image = RImage::MakeFromPixels(info, *fmt, &cons);
        }
    }

    if (!image)
    {
        res.postError(0, "Failed to create image from single pixel buffer (Louvre's fault)");
        return;
    }

    new LSinglePixelBuffer(res.client(), res.version(), id, image);
}


