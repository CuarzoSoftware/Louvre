#include <private/LPositionerPrivate.h>

#include <LClient.h>
#include <LCompositor.h>

using namespace Louvre;

LPositioner::LPositioner()
{
    m_imp = new LPositionerPrivate();
}

LPositioner::~LPositioner()
{
    delete m_imp;
}

LClient *LPositioner::client() const
{
    return m_imp->data.client;
}

wl_resource *LPositioner::resource() const
{
    return m_imp->data.resource;
}

const LSize &LPositioner::sizeS() const
{
    return m_imp->data.sizeS;
}

const LSize &LPositioner::sizeC() const
{
    return m_imp->data.sizeC;
}

const LRect &LPositioner::anchorRectS() const
{
    return m_imp->data.anchorRectS;
}

const LRect &LPositioner::anchorRectC() const
{
    return m_imp->data.anchorRectC;
}

const LPoint &LPositioner::offsetS() const
{
    return m_imp->data.offsetS;
}

const LPoint &LPositioner::offsetC() const
{
    return m_imp->data.offsetC;
}

UInt32 LPositioner::anchor() const
{
    return m_imp->data.anchor;
}

UInt32 LPositioner::gravity() const
{
    return m_imp->data.gravity;
}

#if LOUVRE_XDG_WM_BASE_VERSION >=3

    bool LPositioner::isReactive() const
    {
        return m_imp->data.isReactive;
    }

    const LSize &LPositioner::parentSizeS() const
    {
        return m_imp->data.parentSizeS;
    }

    const LSize &LPositioner::parentSizeC() const
    {
        return m_imp->data.parentSizeC;
    }

    UInt32 LPositioner::parentConfigureSerial() const
    {
        return m_imp->data.parentConfigureSerial;
    }

#endif

Louvre::LPositioner::ConstraintAdjustments LPositioner::constraintAdjustment() const
{
    return m_imp->data.constraintAdjustment;
}

void Louvre::LPositioner::LPositionerPrivate::updateGlobalScale()
{
    data.sizeC = data.sizeS * data.client->compositor()->globalScale();
    data.anchorRectC = data.anchorRectS * data.client->compositor()->globalScale();
    data.offsetC = data.offsetS * data.client->compositor()->globalScale();
    data.parentSizeC = data.parentSizeS * data.client->compositor()->globalScale();
}

LPositioner::LPositionerPrivate *LPositioner::imp() const
{
    return m_imp;
}

