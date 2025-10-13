#include <CZ/Louvre/Backends/LBackend.h>
#include <CZ/Louvre/Private/LSeatPrivate.h>
#include <CZ/Louvre/Private/LPointerPrivate.h>
#include <CZ/Louvre/Private/LKeyboardPrivate.h>
#include <CZ/Louvre/Private/LCompositorPrivate.h>
#include <CZ/Louvre/Private/LOutputPrivate.h>
#include <CZ/Louvre/Private/LFactory.h>

#include <CZ/Louvre/Protocols/Wayland/GSeat.h>

#include <CZ/Louvre/Roles/LToplevelRole.h>
#include <CZ/Louvre/Roles/LSubsurfaceRole.h>
#include <CZ/Louvre/Roles/LPopupRole.h>
#include <CZ/Louvre/Roles/LLayerRole.h>

#include <CZ/Louvre/Cursor/LCursor.h>
#include <CZ/Louvre/Roles/LSurface.h>
#include <CZ/Louvre/Seat/LOutput.h>
#include <CZ/Louvre/LClient.h>
#include <CZ/Louvre/LLog.h>

#include <CZ/Core/CZTime.h>

#include <xkbcommon/xkbcommon-compat.h>
#include <xkbcommon/xkbcommon-compose.h>
#include <xkbcommon/xkbcommon-keysyms.h>
#include <xkbcommon/xkbcommon-names.h>
#include <sys/mman.h>

#include <unistd.h>
#include <fcntl.h>
#include <libudev.h>
#include <libinput.h>
#include <poll.h>
#include <sys/eventfd.h>
#include <cassert>

using namespace CZ;

LSeat::LSeat(const void *params) : LFactoryObject(FactoryObjectType), LPRIVATE_INIT_UNIQUE(LSeat)
{
    assert(params != nullptr && "Invalid parameter passed to LSeat constructor.");
    LSeat**ptr { (LSeat**) params };
    assert(*ptr == nullptr && *ptr == seat() && "Only a single LSeat instance can exist.");
    *ptr = this;

    LFactory::createObject<LDND>(&m_dnd);
    LFactory::createObject<LPointer>(&m_pointer);
    LFactory::createObject<LKeyboard>(&m_keyboard);
    LFactory::createObject<LTouch>(&m_touch);
    LFactory::createObject<LClipboard>(&m_clipboard);
    imp()->enabled = true;
}

LSeat::~LSeat() noexcept
{
    notifyDestruction();

    if (imp()->libseatHandle)
    {
        libseat_close_seat(imp()->libseatHandle);
        imp()->libseatHandle = nullptr;
    }
}

static LSurface *SurfaceAtSubsurfacesVector(const std::vector<LSubsurfaceRole*> &list, SkIPoint point) noexcept
{
    LSurface *ret {};

    for (auto it = list.rbegin(); it != list.rend(); it++)
    {
        if (!(*it)->surface()->mapped())
            continue;

        if ((ret = SurfaceAtSubsurfacesVector((*it)->surface()->subsurfacesAbove(), point)))
            return ret;

        if ((*it)->surface()->inputRegionContainsGlobalPoint(point))
            return (*it)->surface();

        if ((ret = SurfaceAtSubsurfacesVector((*it)->surface()->subsurfacesBelow(), point)))
            return ret;
    }

    return nullptr;
}

static LSurface *SurfaceAtTree(LSurface *root, SkIPoint point)
{
    if (!root->mapped())
        return nullptr;

    LSurface *ret {};

    switch (root->roleId())
    {
    case LSurface::Role::Toplevel:
    {
        if (root->toplevel()->isMinimized())
            return ret;

        for (auto TL = root->toplevel()->childToplevels().rbegin(); TL != root->toplevel()->childToplevels().rend(); TL++)
            if ((ret = SurfaceAtTree((*TL)->surface(), point)))
                return ret;

        for (auto PUP = root->toplevel()->childPopups().rbegin(); PUP != root->toplevel()->childPopups().rend(); PUP++)
            if ((ret = SurfaceAtTree((*PUP)->surface(), point)))
                return ret;

        if ((ret = SurfaceAtSubsurfacesVector(root->subsurfacesAbove(), point)))
            return ret;

        if (root->inputRegionContainsGlobalPoint(point))
            return root;

        if ((ret = SurfaceAtSubsurfacesVector(root->subsurfacesBelow(), point)))
            return ret;

        break;
    }
    case LSurface::Role::Popup:
    {
        for (auto PUP = root->popup()->childPopups().rbegin(); PUP != root->popup()->childPopups().rend(); PUP++)
            if ((ret = SurfaceAtTree((*PUP)->surface(), point)))
                return ret;

        if ((ret = SurfaceAtSubsurfacesVector(root->subsurfacesAbove(), point)))
            return ret;

        if (root->inputRegionContainsGlobalPoint(point))
            return root;

        if ((ret = SurfaceAtSubsurfacesVector(root->subsurfacesBelow(), point)))
            return ret;

        break;
    }
    case LSurface::Role::Layer:
    {
        for (auto PUP = root->layerRole()->childPopups().rbegin(); PUP != root->layerRole()->childPopups().rend(); PUP++)
            if ((ret = SurfaceAtTree((*PUP)->surface(), point)))
                return ret;

        if ((ret = SurfaceAtSubsurfacesVector(root->subsurfacesAbove(), point)))
            return ret;

        if (root->inputRegionContainsGlobalPoint(point))
            return root;

        if ((ret = SurfaceAtSubsurfacesVector(root->subsurfacesBelow(), point)))
            return ret;

        break;
    }
    case LSurface::Role::SessionLock:
    {
        if ((ret = SurfaceAtSubsurfacesVector(root->subsurfacesAbove(), point)))
            return ret;

        if (root->inputRegionContainsGlobalPoint(point))
            return root;

        if ((ret = SurfaceAtSubsurfacesVector(root->subsurfacesBelow(), point)))
            return ret;

        break;
    }
    default:
        break;
    }

    return nullptr;
}

LSurface *LSeat::surfaceAt(SkIPoint point) const noexcept
{
    LSurface *ret { nullptr};

    // Loop layers from overlay to background
    for (auto L = compositor()->layers().rbegin(); L != compositor()->layers().rend(); L++)
    {
        const auto &layer { *L };

        // Loop surfaces from foreground to background
        for (auto S = layer.rbegin(); S != layer.rend(); S++)
        {
            auto *surface { *S };

            // Skip cursors, DND, subsurfaces and popups
            if (surface->roleId() < LSurface::Toplevel || surface->parent())
                continue;

            if ((ret = SurfaceAtTree(surface, point)))
                return ret;
        }
    }

    return ret;
}

const std::vector<LOutput *> &LSeat::outputs() const noexcept
{
    return compositor()->backend()->outputs();
}

const std::set<std::shared_ptr<CZInputDevice>> &LSeat::inputDevices() const noexcept
{
    return compositor()->backend()->inputDevices();
}

const std::vector<LSurface *> &LSeat::idleInhibitorSurfaces() const noexcept
{
    return imp()->idleInhibitors;
}

const std::vector<const LIdleListener *> &LSeat::idleListeners() const noexcept
{
    return imp()->idleListeners;
}

void LSeat::setIsUserIdleHint(bool isIdle) noexcept
{
    imp()->isUserIdleHint = isIdle;
}

bool LSeat::isUserIdleHint() const noexcept
{
    return imp()->isUserIdleHint;
}

const char *LSeat::name() const noexcept
{
    if (imp()->libseatHandle)
        return libseat_seat_name(imp()->libseatHandle);

    return "seat0";
}

LToplevelRole *LSeat::activeToplevel() const noexcept
{
    return imp()->activeToplevelRole;
}

const std::vector<LToplevelResizeSession *> &LSeat::toplevelResizeSessions() const noexcept
{
    return imp()->resizeSessions;
}

const std::vector<LToplevelMoveSession *> &LSeat::toplevelMoveSessions() const noexcept
{
    return imp()->moveSessions;
}

void LSeat::dismissPopups() noexcept
{
    std::list<LSurface*>::const_reverse_iterator s = compositor()->surfaces().rbegin();
    for (; s!= compositor()->surfaces().rend(); s++)
    {
        if ((*s)->popup())
            (*s)->popup()->dismiss();
    }
}

LPopupRole *LSeat::topmostPopup() const noexcept
{
    for (std::list<LSurface*>::const_reverse_iterator it = compositor()->surfaces().crbegin();
         it != compositor()->surfaces().crend(); it++)
    {
        if ((*it)->mapped() && (*it)->popup())
            return (*it)->popup();
    }

    return nullptr;
}

void LSeat::setTTY(UInt32 tty) noexcept
{
    if (imp()->libseatHandle)
        imp()->ttyNumber = tty;
}

Int32 LSeat::openDevice(const char *path, Int32 *fd) noexcept
{
    assert(path);

    if (!imp()->libseatHandle)
        return -1;

    const auto id { libseat_open_device(libseatHandle(), path, fd) };

    if (id == -1)
        LLog(CZError, CZLN, "Failed to open device {}, id {}, fd {}.", path, id, *fd);

    return id;
}

Int32 LSeat::closeDevice(Int32 id) noexcept
{
    if (!imp()->libseatHandle)
        return -1;

    Int32 ret = libseat_close_device(libseatHandle(), id);

    if (ret == -1)
        LLog(CZError, CZLN, "Failed to close device {}", id);

    return ret;
}

libseat *LSeat::libseatHandle() const noexcept
{
    return imp()->libseatHandle;
}

bool LSeat::enabled() const noexcept
{
    return imp()->enabled;
}
