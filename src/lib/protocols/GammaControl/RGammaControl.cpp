#include <protocols/GammaControl/wlr-gamma-control-unstable-v1.h>
#include <protocols/GammaControl/RGammaControl.h>
#include <protocols/Wayland/GOutput.h>
#include <private/LOutputPrivate.h>
#include <LCompositor.h>
#include <LUtils.h>
#include <fcntl.h>

using namespace Louvre::Protocols::GammaControl;

static const struct zwlr_gamma_control_v1_interface imp
{
    .set_gamma = &RGammaControl::set_gamma,
    .destroy = &RGammaControl::destroy,
};

RGammaControl::RGammaControl
    (
        Wayland::GOutput *outputRes,
        Int32 version,
        UInt32 id
    ) noexcept
    :LResource
    (
        outputRes->client(),
        &zwlr_gamma_control_v1_interface,
        version,
        id,
        &imp
    ),
    m_outputRes(outputRes)
{
    outputRes->m_gammaControlRes.emplace_back(this);

    if (!outputRes->output() ||outputRes->output()->gammaSize() == 0)
        failed();
    else
        gammaSize(outputRes->output()->gammaSize());
}

RGammaControl::~RGammaControl()
{
    if (outputRes())
        LVectorRemoveOneUnordered(outputRes()->m_gammaControlRes, this);

    for (LOutput *o : compositor()->outputs())
    {
        if (o->imp()->gammaTable.m_gammaControlResource == this)
        {
            o->imp()->gammaTable.m_gammaControlResource.reset();
            o->setGammaRequest(client(), nullptr);
        }
    }
}

/******************** REQUESTS ********************/

void RGammaControl::destroy(wl_client */*client*/, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

void RGammaControl::set_gamma(wl_client */*client*/, wl_resource *resource, Int32 fd)
{
    auto *res { static_cast<RGammaControl*>(wl_resource_get_user_data(resource)) };

    if (!res->outputRes() || !res->outputRes()->output() || res->outputRes()->output()->gammaSize() == 0)
    {
        res->failed();
        close(fd);
        return;
    }

    LOutput &output { *res->outputRes()->output() };

    const Int32 flags { fcntl(fd, F_GETFL, 0) };

    if (flags == -1)
    {
        res->failed();
        close(fd);
        return;
    }

    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
    {
        res->failed();
        close(fd);
        return;
    }

    LGammaTable gammaTable { output.gammaSize() };
    gammaTable.m_gammaControlResource.reset(res);

    const ssize_t bytesToRead {(ssize_t) (gammaTable.size() * 3 * sizeof(UInt16)) };
    const ssize_t n { pread(fd, gammaTable.red(), bytesToRead, 0) };
    close(fd);

    if (n < 0)
    {
        res->failed();
        return;
    }
    else if (n != bytesToRead)
    {
        res->postError(
            ZWLR_GAMMA_CONTROL_V1_ERROR_INVALID_GAMMA,
            "Invalid gamma size");
        return;
    }

    LWeak<Protocols::GammaControl::RGammaControl> weakRef { res };

    output.setGammaRequest(res->client(), &gammaTable);

    if (!weakRef && output.imp()->gammaTable.m_gammaControlResource != res)
        res->failed();
}

/******************** EVENTS ********************/

void RGammaControl::gammaSize(UInt32 size) noexcept
{
    zwlr_gamma_control_v1_send_gamma_size(resource(), size);
}

void RGammaControl::failed() noexcept
{
    zwlr_gamma_control_v1_send_failed(resource());
}
