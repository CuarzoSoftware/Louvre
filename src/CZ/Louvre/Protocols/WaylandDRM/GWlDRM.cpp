#include <CZ/Louvre/Protocols/WaylandDRM/wayland-drm.h>
#include <CZ/Louvre/Protocols/WaylandDRM/GWlDRM.h>
#include <CZ/Louvre/Protocols/LinuxDMABuf/LDMABuffer.h>
#include <CZ/Louvre/Private/LClientPrivate.h>
#include <CZ/Louvre/Backends/LBackend.h>
#include <CZ/Louvre/LCompositor.h>
#include <CZ/Louvre/LLog.h>
#include <CZ/Core/Utils/CZVectorUtils.h>
#include <CZ/Core/CZBitset.h>
#include <CZ/Ream/RCore.h>
#include <CZ/Ream/RDevice.h>
#include <CZ/Ream/RImage.h>
#include <xf86drm.h>

using namespace CZ;
using namespace CZ::Protocols::WaylandDRM;

static const struct wl_drm_interface imp
{
    .authenticate = &GWlDRM::authenticate,
    .create_buffer = &GWlDRM::create_buffer,
    .create_planar_buffer = &GWlDRM::create_planar_buffer,
    .create_prime_buffer = &GWlDRM::create_prime_buffer
};

LGLOBAL_INTERFACE_IMP(GWlDRM, LOUVRE_WL_DRM_VERSION, wl_drm_interface)

bool GWlDRM::Probe(CZWeak<LGlobal> **slot) noexcept
{
    if (!compositor()->backend()->defaultFeedback())
    {
        LLog(CZWarning, CZLN, "Failed to create {} global (no DMA formats supported by the current backend)", Interface()->name);
        return false;
    }

    if (compositor()->wellKnownGlobals.WlDRM)
    {
        LLog(CZError, CZLN, "Failed to create {} global (already created)", Interface()->name);
        return false;
    }

    *slot = &compositor()->wellKnownGlobals.WlDRM;
    return true;
}

GWlDRM::GWlDRM
    (
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
    this->client()->imp()->wlDRMGlobals.emplace_back(this);

    auto ream { RCore::Get() };
    auto *dev { ream->mainDevice() };

    assert("Do not expose the wl_drm global if there are no DRM nodes available" && dev->drmFd() >= 0);

    drmDevice *drmDev {};

    if (drmGetDevice2(dev->drmFd(), 0, &drmDev) != 0)
        return;

    if (drmDev->available_nodes & (1 << DRM_NODE_RENDER))
    {
        device(drmDev->nodes[DRM_NODE_RENDER]);
        sentRenderNode = true;
    } else
    {
        device(drmDev->nodes[DRM_NODE_PRIMARY]);
        sentRenderNode = false;
    }

    drmFreeDevice(&drmDev);

    if (version >= 2)
        capabilities(WL_DRM_CAPABILITY_PRIME);

    for (const auto &fmt : dev->dmaTextureFormats().formats())
        if (fmt.modifiers().contains(DRM_FORMAT_MOD_INVALID))
            format(fmt.format());
}

GWlDRM::~GWlDRM() noexcept
{
    CZVectorUtils::RemoveOneUnordered(client()->imp()->wlDRMGlobals, this);
}

/******************** REQUESTS ********************/

void GWlDRM::authenticate(wl_client */*client*/, wl_resource *resource, UInt32 magic)
{
    auto &res { *static_cast<GWlDRM*>(wl_resource_get_user_data(resource)) };
    auto ream { RCore::Get() };
    auto *dev { ream->mainDevice() };

    if (!res.sentRenderNode)
    {
        if (drmAuthMagic(dev->drmFd(), magic) != 0)
        {
            res.postError(WL_DRM_ERROR_AUTHENTICATE_FAIL, "Failed to authenticate magic");
            return;
        }
    }

    res.authenticated();
}

void GWlDRM::create_buffer(wl_client */*client*/, wl_resource *resource, UInt32 /*id*/, UInt32 /*name*/, Int32 /*width*/, Int32 /*height*/, UInt32 /*stride*/, UInt32 /*format*/)
{
    auto &res { *static_cast<GWlDRM*>(wl_resource_get_user_data(resource)) };
    res.postError(WL_DRM_ERROR_INVALID_NAME, "Only prime buffers are supported");
}

void GWlDRM::create_planar_buffer(wl_client */*client*/, wl_resource *resource, UInt32 /*id*/, UInt32 /*name*/, Int32 /*width*/, Int32 /*height*/, UInt32 /*format*/, Int32 /*offset0*/, Int32 /*stride0*/, Int32 /*offset1*/, Int32 /*stride1*/, Int32 /*offset2*/, Int32 /*stride2*/)
{
    auto &res { *static_cast<GWlDRM*>(wl_resource_get_user_data(resource)) };
    res.postError(WL_DRM_ERROR_INVALID_NAME, "Only prime buffers are supported");
}

void GWlDRM::create_prime_buffer(wl_client */*client*/, wl_resource *resource, UInt32 id, int fd, Int32 width, Int32 height, UInt32 format, Int32 offset0, Int32 stride0, Int32 /*offset1*/, Int32 /*stride1*/, Int32 /*offset2*/, Int32 /*stride2*/)
{
    auto &res { *static_cast<GWlDRM*>(wl_resource_get_user_data(resource)) };
    RDMABufferInfo info {};
    info.fd[0] = fd;
    info.width = width;
    info.height = height;
    info.format = format;
    info.modifier = DRM_FORMAT_MOD_INVALID;
    info.offset[0] = offset0;
    info.stride[0] = stride0;
    info.planeCount = 1;

    auto ream { RCore::Get() };
    RImageConstraints cons {};
    cons.allocator = ream->mainDevice();
    cons.caps[ream->mainDevice()] = RImageCap_Src;

    auto image { RImage::FromDMA(info, CZOwn::Own, &cons) };

    if (!image)
    {
        res.postError(WL_DRM_ERROR_INVALID_FORMAT, "Failed to import PRIME buffer");
        return;
    }

    new LDMABuffer(std::move(image), res.client(), id);
}

/******************** EVENTS ********************/

void GWlDRM::authenticated() noexcept
{
    wl_drm_send_authenticated(resource());
}

void GWlDRM::device(const char *name) noexcept
{
    wl_drm_send_device(resource(), name);
}

void GWlDRM::format(RFormat format) noexcept
{
    wl_drm_send_format(resource(), format);
}

void GWlDRM::capabilities(UInt32 caps) noexcept
{
    wl_drm_send_capabilities(resource(), caps);
}
