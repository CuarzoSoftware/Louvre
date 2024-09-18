#include <private/LXWindowRolePrivate.h>
#include <private/LSurfacePrivate.h>
#include <private/LXWaylandPrivate.h>
#include <LCursor.h>

LXWindowRole::LXWindowRole(const void *params) noexcept : LBaseSurfaceRole(FactoryObjectType,
                       ((LXWindowRole::Params*)params)->xWaylandSurfaceRes,
                       ((LXWindowRole::Params*)params)->surface,
                       LSurface::Role::XWindow),
    LPRIVATE_INIT_UNIQUE(LXWindowRole)
{
    auto *par { static_cast<const LXWindowRole::Params*>(params) };
    imp()->role = this;
    imp()->winId = par->winId;
    imp()->pos = par->pos;
    imp()->size = par->size;
    imp()->overrideRedirect = imp()->overrideRedirect;
    xWayland()->imp()->windows[imp()->winId] = this;
}

LXWindowRole::~LXWindowRole()
{
    setSurface(nullptr);
    xWayland()->imp()->windows.erase(imp()->winId);
}

const LPoint &LXWindowRole::rolePos() const
{
    return pos();
}

void LXWindowRole::configureRequest(const Configuration &conf)
{
    LOutput *output { cursor()->output() };

    if (output)
    {
        configure(output->pos() + LPoint(200, 200), conf.size);
    }
    else
        configure(conf.pos, conf.size);
}

void LXWindowRole::configure(const LPoint &pos, const LSize &size) noexcept
{
    const LSize prevSize { imp()->size };
    imp()->pos = pos;
    imp()->size = size;

    const UInt32 mask { XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y | XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT | XCB_CONFIG_WINDOW_BORDER_WIDTH };

    const UInt32 values[]
    {
        static_cast<UInt32>(imp()->pos.x()),
        static_cast<UInt32>(imp()->pos.y()),
        static_cast<UInt32>(imp()->size.w()),
        static_cast<UInt32>(imp()->size.h()),
        0
    };

    xcb_configure_window(xWayland()->imp()->conn, imp()->winId, mask, values);

    if (prevSize == imp()->size && !imp()->overrideRedirect)
    {
        const xcb_configure_notify_event_t configure_notify
        {
            .response_type = XCB_CONFIGURE_NOTIFY,
            .event = imp()->winId,
            .window = imp()->winId,
            .x = static_cast<Int16>(imp()->pos.x()),
            .y = static_cast<Int16>(imp()->pos.y()),
            .width = static_cast<UInt16>(imp()->size.w()),
            .height = static_cast<UInt16>(imp()->size.h())
        };

        xWayland()->imp()->sendEventWithSize(
            0,
            imp()->winId,
            XCB_EVENT_MASK_STRUCTURE_NOTIFY,
            &configure_notify,
            sizeof(configure_notify));
    }

    xcb_flush(xWayland()->imp()->conn);
}

const LPoint &LXWindowRole::pos() const noexcept
{
    return imp()->pos;
}

const LSize &LXWindowRole::size() const noexcept
{
    return imp()->size;
}

UInt32 LXWindowRole::winId() const noexcept
{
    return imp()->winId;
}

void LXWindowRole::setSurface(LSurface *newSurface)
{
    if (surface())
    {
        surface()->imp()->setMapped(false);
        surface()->imp()->setPendingRole(nullptr);
        surface()->imp()->applyPendingRole();
        m_surface.reset();
    }

    if (newSurface)
    {
        m_surface.reset(newSurface);
        surface()->imp()->setPendingRole(this);
        surface()->imp()->applyPendingRole();
    }
}

void LXWindowRole::handleSurfaceCommit(CommitOrigin /*origin*/)
{
    const bool mapped { imp()->mapped && surface()->hasBuffer() };
    surface()->imp()->setMapped(mapped);
}
