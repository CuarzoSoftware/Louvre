#include <CZ/Louvre/Protocols/WlrOutputManagement/GWlrOutputManager.h>
#include <CZ/Louvre/Protocols/WlrOutputManagement/RWlrOutputHead.h>
#include <CZ/Louvre/Protocols/DRMLease/RDRMLeaseConnector.h>
#include <CZ/Louvre/Protocols/DRMLease/RDRMLease.h>
#include <CZ/Louvre/Private/LSeatPrivate.h>
#include <CZ/Louvre/Private/LCompositorPrivate.h>
#include <CZ/Louvre/Private/LOutputPrivate.h>
#include <CZ/Louvre/Private/LLockGuard.h>
#include <CZ/Louvre/Backends/LBackend.h>
#include <CZ/Core/CZEventSource.h>
#include <CZ/Louvre/LClient.h>
#include <CZ/Louvre/LLog.h>
#include <unistd.h>
#include <fcntl.h>

void LSeat::LSeatPrivate::seatEnabled(libseat *seat, void *data)
{
    LSeat *lseat = (LSeat*)data;

    lseat->imp()->enabled = true;

    if (compositor()->state() != LCompositor::Initialized)
        return;

    assert(std::this_thread::get_id() == compositor()->imp()->threadId);

    const auto didUnlock { LLockGuard::Unlock() };

    if (compositor()->backend())
        compositor()->backend()->resume();

    if (didUnlock)
        LLockGuard::Lock();

    LLog(CZInfo, CZLN, "Seat {} enabled", libseat_seat_name(seat));
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

    LLog(CZInfo, CZLN, "Seat %s disabled", libseat_seat_name(seat));
    lseat->enabledChanged();

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

    const auto didUnlock { LLockGuard::Unlock() };

    compositor()->backend()->suspend();

    if (didUnlock)
        LLockGuard::Lock();

    libseat_disable_seat(seat);
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

    char *env = getenv("CZ_LOUVRE_ENABLE_LIBSEAT");

    if (env && atoi(env) == 0)
        return false;

    listener.enable_seat = &LSeat::LSeatPrivate::seatEnabled;
    listener.disable_seat = &LSeat::LSeatPrivate::seatDisabled;
    libseatHandle = libseat_open_seat(&listener, seat());

    if (!libseatHandle)
        return false;

    const int fd { libseat_get_fd(libseatHandle) };

    if (fd == -1)
    {
        libseat_close_seat(libseatHandle);
        libseatHandle = nullptr;
        return false;
    }

    libseatEventSource = CZEventSource::Make(fd, EPOLLIN, CZOwn::Borrow, [this](int, UInt32) {
        dispatchSeat();
    });

    const auto lock { LLockGuard() };
    dispatchSeat();
    return true;
}

void LSeat::LSeatPrivate::handleOutputPlugged(LOutput *output) noexcept
{
    seat()->outputPlugged(output);

    for (LClient *c : compositor()->clients())
        for (auto *wlrOutputManager : c->wlrOutputManagerGlobals())
            wlrOutputManager->head(output);
}

void LSeat::LSeatPrivate::handleOutputUnplugged(LOutput *output) noexcept
{
    seat()->outputUnplugged(output);

    if (output->lease())
        output->lease()->finished();

    for (auto *drmConnLeaseRes : output->imp()->drmLeaseConnectorRes)
    {
        drmConnLeaseRes->withdrawn();
        drmConnLeaseRes->done();
    }

    for (auto *head : output->imp()->wlrOutputHeads)
        head->finished();

    compositor()->removeOutput(output);
}

void LSeat::LSeatPrivate::setActiveToplevel(LToplevelRole *newToplevel) noexcept
{
    if (newToplevel == activeToplevelRole)
        return;

    auto *prev { activeToplevelRole.get() };
    activeToplevelRole = newToplevel;
    seat()->activeToplevelChanged(prev);
}
