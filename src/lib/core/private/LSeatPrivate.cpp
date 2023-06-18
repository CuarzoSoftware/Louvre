#include <private/LSeatPrivate.h>
#include <private/LCompositorPrivate.h>
#include <private/LOutputPrivate.h>
#include <LLog.h>
#include <unistd.h>

int LSeat::LSeatPrivate::seatEvent(int, unsigned int, void *data)
{
    LSeat *seat = (LSeat*)data;
    libseat_dispatch(seat->libseatHandle(), 0);
    return 0;
}

void LSeat::LSeatPrivate::seatEnabled(libseat *seat, void *data)
{
    LSeat *lseat = (LSeat*)data;

    lseat->imp()->enabled = true;

    LCompositor::compositor()->imp()->renderMutex.unlock();

    if (compositor()->isGraphicBackendInitialized())
        compositor()->imp()->graphicBackend->resume(compositor());

    if (compositor()->isInputBackendInitialized())
        compositor()->imp()->inputBackend->resume(lseat);

    LCompositor::compositor()->imp()->renderMutex.lock();

    for (LOutput *o : compositor()->outputs())
    {
        if (o->state() == LOutput::Suspended)
            o->imp()->state = LOutput::Initialized;
    }

    LLog::debug("[%s] enabled.", libseat_seat_name(seat));

    lseat->seatEnabled();
}

void LSeat::LSeatPrivate::seatDisabled(libseat *seat, void *data)
{
    LSeat *lseat = (LSeat*)data;

    lseat->imp()->enabled = false;

    for (LOutput *o : compositor()->outputs())
    {
        if (o->state() == LOutput::Initialized)
            o->imp()->state = LOutput::Suspended;
    }

    LCompositor::compositor()->imp()->renderMutex.unlock();

    if (compositor()->isGraphicBackendInitialized())
        compositor()->imp()->graphicBackend->pause(compositor());

    if (compositor()->isInputBackendInitialized())
        compositor()->imp()->inputBackend->suspend(lseat);

    LCompositor::compositor()->imp()->renderMutex.lock();

    libseat_disable_seat(seat);

    LLog::debug("[%s] disabled.", libseat_seat_name(seat));

    lseat->seatDisabled();
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
    libseatHandle = libseat_open_seat(&listener, LCompositor::compositor()->seat());

    if (!libseatHandle)
        return false;

    int fd = libseat_get_fd(libseatHandle);

    if (fd == -1)
    {
        libseat_close_seat(libseatHandle);
        libseatHandle = nullptr;
        return false;
    }

    LCompositor::addFdListener(
        libseat_get_fd(libseatHandle),
        LCompositor::compositor()->seat(),
        &LSeat::LSeatPrivate::seatEvent);

    LCompositor::compositor()->imp()->renderMutex.lock();
    dispatchSeat();
    LCompositor::compositor()->imp()->renderMutex.unlock();

    LLog::debug("[compositor] Using libseat.");
    return true;
}
