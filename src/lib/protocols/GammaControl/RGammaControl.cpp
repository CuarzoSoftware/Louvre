#include <protocols/GammaControl/private/RGammaControlPrivate.h>
#include <protocols/GammaControl/wlr-gamma-control-unstable-v1.h>
#include <protocols/Wayland/GOutput.h>
#include <private/LOutputPrivate.h>
#include <LCompositor.h>

using namespace Louvre;

static struct zwlr_gamma_control_v1_interface gamma_control_implementation =
{
    .set_gamma = &RGammaControl::RGammaControlPrivate::set_gamma,
    .destroy = &RGammaControl::RGammaControlPrivate::destroy,
};

RGammaControl::RGammaControl
    (
        Wayland::GOutput *gOutput,
        Int32 version,
        UInt32 id
    )
    :LResource
    (
        gOutput->client(),
        &zwlr_gamma_control_v1_interface,
        version,
        id,
        &gamma_control_implementation
    ),
    LPRIVATE_INIT_UNIQUE(RGammaControl)
{
    imp()->gOutput.reset(gOutput);
    gOutput->m_gammaControlRes.push_back(this);

    if (!outputGlobal()->output() || outputGlobal()->output()->gammaSize() == 0)
        failed();
    else
        gammaSize(outputGlobal()->output()->gammaSize());
}

RGammaControl::~RGammaControl()
{
    if (outputGlobal())
        LVectorRemoveOneUnordered(imp()->gOutput.get()->m_gammaControlRes, this);

    for (LOutput *o : compositor()->outputs())
    {
        if (o->imp()->gammaTable.m_gammaControlResource.get() == this)
        {
            o->imp()->gammaTable.m_gammaControlResource.reset();
            o->setGammaRequest(client(), nullptr);
        }
    }
}

Protocols::Wayland::GOutput *RGammaControl::outputGlobal() const
{
    return imp()->gOutput.get();
}

bool RGammaControl::gammaSize(UInt32 size)
{
    zwlr_gamma_control_v1_send_gamma_size(resource(), size);
    return true;
}

bool RGammaControl::failed()
{
    zwlr_gamma_control_v1_send_failed(resource());
    return true;
}
