#include "LDRM.h"
#include "LDRMConnector.h"
#include "LDRMDevice.h"
#include "LDRMCrtc.h"
#include "LDRMEncoder.h"
#include "LDRMPlane.h"

#include <cstring>
#include <fcntl.h>
#include <libudev.h>
#include <unistd.h>
#include <xf86drm.h>
#include <xf86drmMode.h>

static const struct
{
    const char *name;
    uint64_t cap;
}
client_caps[] =
{
    { "STEREO_3D", DRM_CLIENT_CAP_STEREO_3D },
    { "UNIVERSAL_PLANES", DRM_CLIENT_CAP_UNIVERSAL_PLANES },
    { "ATOMIC", DRM_CLIENT_CAP_ATOMIC },
    { "ASPECT_RATIO", DRM_CLIENT_CAP_ASPECT_RATIO },
    { "WRITEBACK_CONNECTORS", DRM_CLIENT_CAP_WRITEBACK_CONNECTORS },
};

static const struct
{
    const char *name;
    uint64_t cap;
}
caps[] =
{
    { "DUMB_BUFFER", DRM_CAP_DUMB_BUFFER },
    { "VBLANK_HIGH_CRTC", DRM_CAP_VBLANK_HIGH_CRTC },
    { "DUMB_PREFERRED_DEPTH", DRM_CAP_DUMB_PREFERRED_DEPTH },
    { "DUMB_PREFER_SHADOW", DRM_CAP_DUMB_PREFER_SHADOW },
    { "PRIME", DRM_CAP_PRIME },
    { "PRIME_IMPORT", DRM_PRIME_CAP_IMPORT },
    { "PRIME_EXPORT", DRM_PRIME_CAP_EXPORT },
    { "TIMESTAMP_MONOTONIC", DRM_CAP_TIMESTAMP_MONOTONIC },
    { "ASYNC_PAGE_FLIP", DRM_CAP_ASYNC_PAGE_FLIP },
    { "CURSOR_WIDTH", DRM_CAP_CURSOR_WIDTH },
    { "CURSOR_HEIGHT", DRM_CAP_CURSOR_HEIGHT },
    { "ADDFB2_MODIFIERS", DRM_CAP_ADDFB2_MODIFIERS },
    { "PAGE_FLIP_TARGET", DRM_CAP_PAGE_FLIP_TARGET },
    { "CRTC_IN_VBLANK_EVENT", DRM_CAP_CRTC_IN_VBLANK_EVENT },
    { "SYNCOBJ", DRM_CAP_SYNCOBJ },
    { "SYNCOBJ_TIMELINE", DRM_CAP_SYNCOBJ_TIMELINE },
};


LDRMDevice::LDRMDevice(LDRM *drm, udev_device *device)
{
    m_drm = drm;
    m_udevDevice = device;
    strcpy(m_name,  udev_device_get_property_value(device, "DEVNAME"));

    m_id = drm->compositor()->seat()->openDevice(name(), &m_fd);

    if(id() < 0 || fd() < 0)
    {
        LLog::error("%s Failed to open device %s.", LBACKEND_NAME, name());
        return;
    }

    if(update())
    {
        m_valid = true;
    }
}

LDRMDevice::~LDRMDevice()
{
    if(m_udevDevice)
        udev_device_unref(m_udevDevice);
}

bool LDRMDevice::update()
{
    if(!updateDriverInfo())
        return false;

    if(!updateResources())
        return false;

    if(!updateCrtcs())
        return false;

    if(!updateEncoders())
        return false;

    if(!updatePlanes())
        return false;

    if(!updateConnectors())
        return false;

    return true;
}

bool LDRMDevice::updateDriverInfo()
{
    drmVersion *version = drmGetVersion(fd());

    LLog::debug("%s Getting driver info for device %s:", LBACKEND_NAME, name());

    if(!version)
    {
        LLog::error("%s Could not get driver version.", LBACKEND_NAME);
        drmFreeVersion(version);
        return false;
    }

    LLog::debug("%s Driver: %s (%s) version %d.%d.%d (%s)",
           LBACKEND_NAME,
           version->name,
           version->desc,
           version->version_major,
           version->version_minor,
           version->version_patchlevel,
           version->date);


    drmFreeVersion(version);


    LLog::debug("%s Client capabilities:", LBACKEND_NAME);

    for(size_t i = 0; i < sizeof(client_caps) / sizeof(client_caps[0]); i++)
    {
        bool supported = drmSetClientCap(fd(), client_caps[i].cap, 1) == 0;
        LLog::debug("%s\tCLIENT_CAP_%s = %s", LBACKEND_NAME, client_caps[i].name, supported ? "Yes" : "No");
    }

    LLog::debug("%s Capabilities:", LBACKEND_NAME);

    for(size_t i = 0; i < sizeof(caps) / sizeof(caps[0]); i++)
    {
        UInt64 cap;

        if(drmGetCap(fd(), caps[i].cap, &cap) == 0)
        {
            LLog::debug("%s \tCAP_%s = %lu", LBACKEND_NAME, caps[i].name, cap);

            if(caps[i].cap == DRM_CAP_DUMB_BUFFER)
               m_capDumbBuffer = cap == 1;
            else if(caps[i].cap == DRM_CAP_ADDFB2_MODIFIERS)
               m_capAddFB2Mod = cap == 1;
            else if(caps[i].cap == DRM_PRIME_CAP_IMPORT)
               m_capPrimeImport = cap == 1;
            else if(caps[i].cap == DRM_PRIME_CAP_EXPORT)
               m_capPrimeExport = cap == 1;
        }

    }

    return true;
}

bool LDRMDevice::updateResources()
{
    if(resources())
    {
        drmModeFreeResources(m_resources);
        m_resources = nullptr;
    }

    m_resources = drmModeGetResources(fd());

    if(!resources())
    {
        LLog::error("%s Could not get device resources.", LBACKEND_NAME);
        return false;
    }

    return true;
}

bool LDRMDevice::updateConnectors()
{
    if(!resources())
        return false;

    for(int i = 0; i < resources()->count_connectors; i++)
    {
        drmModeConnector *c = drmModeGetConnector(fd(), resources()->connectors[i]);

        if(!c)
        {
            LLog::error("%s Failed to get connector for device %s.", LBACKEND_NAME, name());
            return false;
        }

        LDRMConnector *connector = findConnector(resources()->connectors[i]);

        if(connector)
        {
            connector->setDRMResouce(c);
            continue;
        }

        connector = new LDRMConnector(this, c);
    }

    if(connectors().empty())
    {
        LLog::warning("%s No connector avaliable for device %s.", LBACKEND_NAME, name());
        return false;
    }
    else
    {
        LLog::debug("%s Found %lu connectors for %s:", LBACKEND_NAME, crtcs().size(), name());
    }

    return true;
}

bool LDRMDevice::updatePlanes()
{

    drmModePlaneRes *planeResources = drmModeGetPlaneResources(fd());

    if(!planeResources)
    {
        LLog::error("%s Could not get device planes.", LBACKEND_NAME);
        return false;
    }

    for(UInt32 i = 0; i < planeResources->count_planes; i++)
    {

        drmModePlane *planeResource = drmModeGetPlane(fd(), planeResources->planes[i]);

        if(!planeResource)
        {
            LLog::error("%s Could not get info of plane (%d):", LBACKEND_NAME, planeResources->planes[i]);
            continue;
        }

        LDRMPlane *plane = findPlane(planeResource->plane_id);

        if(plane)
            plane->setDRMResource(planeResource);
        else
            plane = new LDRMPlane(this, planeResource);

        plane->updateProperties();
        plane->updatePossibleCrtcs();

    }

    drmModeFreePlaneResources(planeResources);

    if(planes().empty())
    {
        LLog::warning("%s No planes avaliable for device %s.", LBACKEND_NAME, name());
        return false;
    }
    else
    {
        LLog::debug("%s Found %lu planes info for %s:", LBACKEND_NAME, planes().size(), name());
    }

    return true;
}

bool LDRMDevice::updateCrtcs()
{
    if(!resources())
        return false;

    for(int i = 0; i < resources()->count_crtcs; i++)
    {
        drmModeCrtc * c = drmModeGetCrtc(fd(), resources()->crtcs[i]);

        if(!c)
        {
            LLog::error("%s Failed to get crtc for device %s.", LBACKEND_NAME, name());
            return false;
        }

        LDRMCrtc *crtc = findCrtc(resources()->crtcs[i]);

        if(crtc)
        {
            crtc->setDRMResouce(c);
            continue;
        }

        crtc = new LDRMCrtc(this, c);
    }

    if(crtcs().empty())
    {
        LLog::warning("%s No crtcs avaliable for device %s.", LBACKEND_NAME, name());
        return false;
    }
    else
    {
        LLog::debug("%s Found %lu crtcs for %s:", LBACKEND_NAME, crtcs().size(), name());
    }

    return true;
}

bool LDRMDevice::updateEncoders()
{
    if(!resources())
        return false;

    LDRMEncoder *encoder;
    drmModeEncoder *encoderResource;

    for(int i = 0; i < resources()->count_encoders; i++)
    {
        encoderResource = drmModeGetEncoder(fd(), resources()->encoders[i]);
        encoder = findEncoder(resources()->encoders[i]);

        if(encoder)
        {
            encoder->setDRMResouce(encoderResource);
            continue;
        }

        encoder = new LDRMEncoder(this, encoderResource);
    }

    /* We update it after because we need all the encoders first */
    for(LDRMEncoder *e : encoders())
    {
        e->updatePossibleClones();
        e->updatePossibleCrtcs();
    }

    if(encoders().empty())
    {
        LLog::warning("%s \tNo encoders avaliable for device %s.", LBACKEND_NAME, name());
        return false;
    }
    else
    {
        LLog::debug("%s Found %lu encoders for %s:", LBACKEND_NAME, encoders().size(), name());
    }

    return true;
}

int LDRMDevice::id() const
{
    return m_id;
}

int LDRMDevice::fd() const
{
    return m_fd;
}

bool LDRMDevice::valid() const
{
    return m_valid;
}

const char *LDRMDevice::name() const
{
    return m_name;
}

LDRM *LDRMDevice::drm() const
{
    return m_drm;
}

udev_device *LDRMDevice::udevDevice() const
{
    return m_udevDevice;
}

drmModeRes *LDRMDevice::resources() const
{
    return m_resources;
}

list<LDRMPlane *> &LDRMDevice::planes()
{
    return m_planes;
}

list<LDRMCrtc *> &LDRMDevice::crtcs()
{
    return m_crtcs;
}

list<LDRMEncoder *> &LDRMDevice::encoders()
{
    return m_encoders;
}

list<LDRMConnector *> &LDRMDevice::connectors()
{
    return m_connectors;
}

LDRMConnector *LDRMDevice::findConnector(UInt32 id)
{
    for(LDRMConnector *c : connectors())
    {
        if(c->id() == id)
            return c;
    }

    return nullptr;
}

LDRMCrtc *LDRMDevice::findCrtc(UInt32 id)
{
    for(LDRMCrtc *c : crtcs())
    {
        if(c->id() == id)
            return c;
    }

    return nullptr;
}

LDRMEncoder *LDRMDevice::findEncoder(UInt32 id)
{
    for(LDRMEncoder *e : encoders())
    {
        if(e->id() == id)
            return e;
    }

    return nullptr;
}

LDRMPlane *LDRMDevice::findPlane(UInt32 id)
{
    for(LDRMPlane *p : planes())
    {
        if(p->id() == id)
            return p;
    }

    return nullptr;
}

bool LDRMDevice::capDumbBuffer() const
{
    return m_capDumbBuffer;
}

bool LDRMDevice::capAddFB2Mod() const
{
    return m_capAddFB2Mod;
}

bool LDRMDevice::capPrimeImport() const
{
    return m_capPrimeImport;
}

bool LDRMDevice::capPrimeExport() const
{
    return m_capPrimeExport;
}
