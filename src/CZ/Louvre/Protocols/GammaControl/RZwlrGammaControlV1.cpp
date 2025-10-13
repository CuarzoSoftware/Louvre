#include <CZ/Louvre/Protocols/GammaControl/wlr-gamma-control-unstable-v1.h>
#include <CZ/Louvre/Protocols/GammaControl/RZwlrGammaControlV1.h>
#include <CZ/Louvre/Protocols/Wayland/GOutput.h>
#include <CZ/Louvre/Private/LOutputPrivate.h>
#include <CZ/Louvre/LCompositor.h>
#include <CZ/Ream/RGammaLUT.h>
#include <CZ/Core/Utils/CZVectorUtils.h>
#include <fcntl.h>

using namespace CZ::Protocols::GammaControl;

static const struct zwlr_gamma_control_v1_interface imp
{
    .set_gamma = &RZwlrGammaControlV1::set_gamma,
    .destroy = &RZwlrGammaControlV1::destroy,
};

RZwlrGammaControlV1::RZwlrGammaControlV1
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

    if (!outputRes->output() || outputRes->output()->gammaSize() == 0)
        failed();
    else
        gammaSize(outputRes->output()->gammaSize());
}

RZwlrGammaControlV1::~RZwlrGammaControlV1()
{
    if (outputRes())
        CZVectorUtils::RemoveOneUnordered(outputRes()->m_gammaControlRes, this);

    for (LOutput *o : compositor()->outputs())
    {
        if (o->imp()->wlrGammaControl == this)
        {
            o->imp()->wlrGammaControl.reset();
            o->setGammaRequest(client(), nullptr);
        }
    }
}

/******************** REQUESTS ********************/

void RZwlrGammaControlV1::destroy(wl_client */*client*/, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

void RZwlrGammaControlV1::set_gamma(wl_client */*client*/, wl_resource *resource, Int32 fd)
{
    auto *res { static_cast<RZwlrGammaControlV1*>(wl_resource_get_user_data(resource)) };

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

    auto gammaLUT { RGammaLUT::Make(output.gammaSize()) };
    const ssize_t bytesToRead {(ssize_t) (gammaLUT->size() * 3 * sizeof(UInt16)) };
    const ssize_t n { pread(fd, gammaLUT->red().data(), bytesToRead, 0) };
    close(fd);

    if (n < 0)
    {
        res->failed();
        return;
    }
    else if (n != bytesToRead)
    {
        res->postError(ZWLR_GAMMA_CONTROL_V1_ERROR_INVALID_GAMMA, "Failed to read gamma LUT from fd {}/{} bytes", n, bytesToRead);
        return;
    }

    output.setGammaRequest(res->client(), gammaLUT);

    if (output.gammaLUT() == gammaLUT)
        output.imp()->wlrGammaControl.reset(res);
    else
        res->failed();
}

/******************** EVENTS ********************/

void RZwlrGammaControlV1::gammaSize(UInt32 size) noexcept
{
    zwlr_gamma_control_v1_send_gamma_size(resource(), size);
}

void RZwlrGammaControlV1::failed() noexcept
{
    zwlr_gamma_control_v1_send_failed(resource());
}
