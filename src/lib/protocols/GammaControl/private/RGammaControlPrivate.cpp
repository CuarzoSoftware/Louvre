#include <fcntl.h>
#include <protocols/GammaControl/private/RGammaControlPrivate.h>
#include <protocols/GammaControl/wlr-gamma-control-unstable-v1.h>
#include <protocols/Wayland/GOutput.h>
#include <private/LOutputPrivate.h>

void RGammaControl::RGammaControlPrivate::resource_destroy(wl_resource *resource)
{
    delete (RGammaControl*)wl_resource_get_user_data(resource);
}

void RGammaControl::RGammaControlPrivate::destroy(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);
    wl_resource_destroy(resource);
}

void RGammaControl::RGammaControlPrivate::set_gamma(wl_client *client, wl_resource *resource, Int32 fd)
{
    L_UNUSED(client)

    RGammaControl *rGammaControl { (RGammaControl*)wl_resource_get_user_data(resource) };

    if (!rGammaControl->outputGlobal() || !rGammaControl->outputGlobal()->output() || rGammaControl->outputGlobal()->output()->gammaSize() == 0)
    {
        rGammaControl->failed();
        close(fd);
        return;
    }

    LOutput *output = rGammaControl->outputGlobal()->output();

    Int32 flags = fcntl(fd, F_GETFL, 0);

    if (flags == -1)
    {
        rGammaControl->failed();
        close(fd);
        return;
    }

    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
    {
        rGammaControl->failed();
        close(fd);
        return;
    }

    LGammaTable gammaTable {rGammaControl->outputGlobal()->output()->gammaSize()};
    gammaTable.m_gammaControlResource = rGammaControl;

    ssize_t bytesToRead = gammaTable.size() * 3 * sizeof(UInt16);
    ssize_t n = pread(fd, gammaTable.red(), bytesToRead, 0);
    close(fd);

    if (n < 0)
    {
        rGammaControl->failed();
        return;
    }
    else if (n != bytesToRead)
    {
        wl_resource_post_error(rGammaControl->resource(),
                               ZWLR_GAMMA_CONTROL_V1_ERROR_INVALID_GAMMA,
                               "Invalid gamma size");
        return;
    }

    const bool *isResAlive = rGammaControl->isAlive();

    output->setGammaRequest(rGammaControl->client(), &gammaTable);

    if (*isResAlive && output->imp()->gammaTable.m_gammaControlResource != rGammaControl)
        rGammaControl->failed();
}
