#include "LDRMPlane.h"
#include "LDRM.h"
#include "LDRMCrtc.h"
#include <cstring>

LDRMPlane::LDRMPlane(LDRMDevice *device, drmModePlane *plane)
{
    m_device = device;
    m_plane = plane;
    device->planes().push_back(this);
}

LDRMPlane::~LDRMPlane()
{
    if(m_plane)
        drmModeFreePlane(m_plane);

    device()->planes().remove(this);
}

bool LDRMPlane::updateProperties()
{

    drmModeObjectProperties *planeProperties = drmModeObjectGetProperties(device()->fd(), id(), DRM_MODE_OBJECT_PLANE);

    if(!planeProperties)
    {
        LLog::error("%s Failed to get properties of plane (%d).", LBACKEND_NAME, id());
        return false;
    }

    for(UInt32 i = 0; i < planeProperties->count_props; i++)
    {
        drmModePropertyRes *prop = drmModeGetProperty(device()->fd(), planeProperties->props[i]);

        if (!prop)
        {
            LLog::error("%s Failed to get property (%d) of plane (%d).", LBACKEND_NAME, planeProperties->props[i], id());
            continue;
        }

        if(strcmp("type", prop->name) == 0)
        {
            m_type = planeProperties->prop_values[i];
            if(m_type == DRM_PLANE_TYPE_CURSOR)
            {
                //LLog("%s \t Type: %s\n", LBACKEND_NAME, "CURSOR");
            }
            else if(m_type == DRM_PLANE_TYPE_PRIMARY)
            {
                //LLog("%s \t Type: %s\n", LBACKEND_NAME, "PRIMARY");
            }
            else if(m_type == DRM_PLANE_TYPE_OVERLAY)
            {
                //LLog("%s \t Type: %s\n", LBACKEND_NAME, "OVERLAY");
            }
            else
            {
                m_type = -1;
                //LLog("%s \t Type: %s\n", LBACKEND_NAME, "UNKNOW");
            }
        }
        else if(strcmp("IN_FORMATS", prop->name) == 0)
        {
            drmModePropertyBlobRes *blob = drmModeGetPropertyBlob(device()->fd(), planeProperties->prop_values[i]);

            if(!blob)
            {
                LLog::error("%s \t Could not get IN_FORMATS blob.", LBACKEND_NAME);
                goto freeProp;
            }

            inFormats().clear();

            drmModeFormatModifierIterator iter;
            LDRMInFormat inFormat;

            while (drmModeFormatModifierBlobIterNext(blob, &iter))
            {
                inFormat.format = iter.fmt;
                inFormat.mod = iter.mod;
                inFormats().push_back(inFormat);
            }

            drmModeFreePropertyBlob(blob);

        }

        freeProp:
        drmModeFreeProperty(prop);
    }

    drmModeFreeObjectProperties(planeProperties);

    return true;
}

bool LDRMPlane::updatePossibleCrtcs()
{
    possibleCrtcs().clear();

    //LLog::debug("%s Getting possible CRTCs for plane (%d):", LBACKEND_NAME, id());

    for(int i = 0; i < device()->resources()->count_crtcs; i++)
    {
        int mask = 1 << i;

        if(plane()->possible_crtcs & mask)
        {
            LDRMCrtc *crtc = device()->findCrtc(device()->resources()->crtcs[i]);

            if(crtc)
            {
                possibleCrtcs().push_back(crtc);
                //LLog::debug("%s \tCRTC (%d)", LBACKEND_NAME, crtc->id());
            }
        }
    }

    if(possibleCrtcs().empty())
    {
        //LLog::debug("%s \tNo possible CRTCs found for plane (%d):", LBACKEND_NAME, id());
        return false;
    }

    return true;
}

void LDRMPlane::setDRMResource(drmModePlane *plane)
{
    if(m_plane)
        drmModeFreePlane(m_plane);

    m_plane = plane;
}

Int32 LDRMPlane::type() const
{
    return m_type;
}

UInt32 LDRMPlane::id() const
{
    return plane()->plane_id;
}

LDRMDevice *LDRMPlane::device() const
{
    return m_device;
}

drmModePlane *LDRMPlane::plane() const
{
    return m_plane;
}

list<LDRMCrtc *> &LDRMPlane::possibleCrtcs()
{
    return m_possibleCrtcs;
}

list<LDRMPlane::LDRMInFormat> &LDRMPlane::inFormats()
{
    return m_inFormats;
}
