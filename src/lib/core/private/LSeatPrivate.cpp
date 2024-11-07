#include <protocols/DRMLease/RDRMLease.h>
#include <protocols/DRMLease/RDRMLeaseConnector.h>
#include <private/LSeatPrivate.h>
#include <private/LCompositorPrivate.h>
#include <private/LOutputPrivate.h>
#include <private/LCursorPrivate.h>
#include <LClient.h>
#include <LLog.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>

void LSeat::LSeatPrivate::seatEnabled(libseat *seat, void *data)
{
    LSeat *lseat = (LSeat*)data;

    lseat->imp()->enabled = true;

    if (compositor()->state() != LCompositor::Initialized)
        return;

    compositor()->imp()->unlock();

    if (compositor()->isGraphicBackendInitialized())
        compositor()->imp()->graphicBackend->backendResume();

    compositor()->imp()->lock();

    if (compositor()->isInputBackendInitialized())
        compositor()->imp()->inputBackend->backendResume();

    // Restore Wayland and Aux events
    epoll_ctl(compositor()->imp()->epollFd,
              EPOLL_CTL_ADD,
              compositor()->imp()->events[LEV_WAYLAND].data.fd,
              &compositor()->imp()->events[LEV_WAYLAND]);

    epoll_ctl(compositor()->imp()->epollFd,
              EPOLL_CTL_ADD,
              compositor()->imp()->events[LEV_AUX].data.fd,
              &compositor()->imp()->events[LEV_AUX]);

    LLog::debug("[LSeatPrivate::seatEnabled] %s enabled.", libseat_seat_name(seat));
    lseat->enabledChanged();
}

void LSeat::LSeatPrivate::seatDisabled(libseat *seat, void *data)
{
    LSeat *lseat = (LSeat*)data;

    if (!lseat->imp()->enabled)
        return;

    lseat->imp()->enabled = false;

    if (compositor()->state() != LCompositor::Initialized)
        return;

    for (LOutput *output : lseat->outputs())
    {
        if (output->lease())
            output->lease()->finished();

        for (auto *drmConnLeaseRes : output->imp()->drmLeaseConnectorRes)
        {
            drmConnLeaseRes->withdrawn();
            drmConnLeaseRes->done();
        }
    }

    if (compositor()->isInputBackendInitialized())
        compositor()->imp()->inputBackend->backendSuspend();

    compositor()->imp()->unlock();

    if (compositor()->isGraphicBackendInitialized())
        compositor()->imp()->graphicBackend->backendSuspend();

    compositor()->imp()->lock();

    libseat_disable_seat(seat);

    // Disable Wayland and Aux events
    epoll_ctl(compositor()->imp()->epollFd,
              EPOLL_CTL_DEL,
              compositor()->imp()->events[LEV_WAYLAND].data.fd,
              NULL);

    epoll_ctl(compositor()->imp()->epollFd,
              EPOLL_CTL_DEL,
              compositor()->imp()->events[LEV_AUX].data.fd,
              NULL);

    LLog::debug("[LSeatPrivate::seatDisabled] %s disabled.", libseat_seat_name(seat));

    lseat->enabledChanged();
}

void LSeat::LSeatPrivate::dispatchSeat()
{
    if (libseatHandle)
        while (libseat_dispatch(libseatHandle, 0) > 0);
}

bool LSeat::LSeatPrivate::initLibseat()
{
    if (libseatHandle)
        return true;

    char *env = getenv("LOUVRE_ENABLE_LIBSEAT");

    if (env && atoi(env) == 0)
        return false;

    listener.enable_seat = &LSeat::LSeatPrivate::seatEnabled;
    listener.disable_seat = &LSeat::LSeatPrivate::seatDisabled;
    libseatHandle = libseat_open_seat(&listener, seat());

    if (!libseatHandle)
        return false;

    int fd = libseat_get_fd(libseatHandle);

    if (fd == -1)
    {
        libseat_close_seat(libseatHandle);
        libseatHandle = nullptr;
        return false;
    }

    compositor()->imp()->events[LEV_LIBSEAT].events = EPOLLIN;
    compositor()->imp()->events[LEV_LIBSEAT].data.fd = fd;

    epoll_ctl(compositor()->imp()->epollFd,
              EPOLL_CTL_ADD,
              compositor()->imp()->events[LEV_LIBSEAT].data.fd,
              &compositor()->imp()->events[LEV_LIBSEAT]);

    compositor()->imp()->lock();
    dispatchSeat();
    compositor()->imp()->unlock();

    LLog::debug("[LSeatPrivate::initLibseat] Using libseat.");
    return true;
}

void LSeat::LSeatPrivate::backendOutputPlugged(LOutput *output)
{
    if (enabled)
        seat()->outputPlugged(output);
}

void LSeat::LSeatPrivate::backendOutputUnplugged(LOutput *output)
{
    if (enabled)
    {
        seat()->outputUnplugged(output);

        if (output->lease())
            output->lease()->finished();

        for (auto *drmConnLeaseRes : output->imp()->drmLeaseConnectorRes)
        {
            drmConnLeaseRes->withdrawn();
            drmConnLeaseRes->done();
        }
    }
}
