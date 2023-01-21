#include "LDRMEncoder.h"
#include "LDRM.h"
#include "LDRMDevice.h"

LDRMEncoder::LDRMEncoder(LDRMDevice *device, drmModeEncoder *encoder)
{
    m_device = device;
    m_encoder = encoder;
    device->encoders().push_back(this);
}

LDRMEncoder::~LDRMEncoder()
{
    if(m_encoder)
        drmModeFreeEncoder(m_encoder);

    device()->encoders().remove(this);
}

UInt32 LDRMEncoder::id() const
{
    return encoder()->encoder_id;
}

drmModeEncoder *LDRMEncoder::encoder() const
{
    return m_encoder;
}

LDRMDevice *LDRMEncoder::device() const
{
    return m_device;
}

const char *LDRMEncoder::typeStr() const
{
    switch(encoder()->encoder_type)
    {
        case DRM_MODE_ENCODER_DPI:
            return "DPI";
        break;
        case DRM_MODE_ENCODER_LVDS:
            return "LVDS";
        break;
        case DRM_MODE_ENCODER_DAC:
            return "DAC";
        break;
        case DRM_MODE_ENCODER_TVDAC:
            return "TVDAC";
        break;
        case DRM_MODE_ENCODER_DPMST:
            return "DPMST";
        break;
        case DRM_MODE_ENCODER_DSI:
            return "DSI";
        break;
        case DRM_MODE_ENCODER_TMDS:
            return "TMDS";
        break;
        case DRM_MODE_ENCODER_VIRTUAL:
            return "VIRTUAL";
        break;
        case DRM_MODE_ENCODER_NONE:
            return "NONE";
        break;
    }

    return "UNKNOWN";
}

void LDRMEncoder::setDRMResouce(drmModeEncoder *encoder)
{
    if(m_encoder)
        drmModeFreeEncoder(m_encoder);

    m_encoder = encoder;
}

list<LDRMEncoder *> &LDRMEncoder::possibleClones()
{
    return m_possibleClones;
}

list<LDRMCrtc *> &LDRMEncoder::possibleCrtcs()
{
    return m_possibleCrts;
}

bool LDRMEncoder::updatePossibleClones()
{
    possibleClones().clear();

    //LLog::debug("%s Getting possible clones for encoder (%d):", LBACKEND_NAME, id());

    for(int i = 0; i < device()->resources()->count_encoders; i++)
    {
        int mask = 1 << i;

        if(encoder()->possible_clones & mask && device()->resources()->encoders[i] != id())
        {
            LDRMEncoder *clone = device()->findEncoder(device()->resources()->encoders[i]);

            if(clone)
            {
                possibleClones().push_back(clone);
                //LLog::debug("%s \tEncoder (%d)", LBACKEND_NAME, clone->id());
            }
        }
    }

    if(possibleClones().empty())
    {
        //LLog::debug("%s \tNo possible clones found for encoder (%d):", LBACKEND_NAME, id());
        return false;
    }

    return true;
}

bool LDRMEncoder::updatePossibleCrtcs()
{
    possibleCrtcs().clear();

    //LLog::debug("%s Getting possible CRTCs for encoder (%d):", LBACKEND_NAME, id());

    for(int i = 0; i < device()->resources()->count_crtcs; i++)
    {
        int mask = 1 << i;

        if(encoder()->possible_crtcs & mask)
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
        LLog::debug("%s \tNo possible CRTCs found for encoder (%d):", LBACKEND_NAME, id());
        return false;
    }

    return true;
}
