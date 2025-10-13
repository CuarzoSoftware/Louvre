#include "RSync.h"
#include <CZ/Louvre/Protocols/ForeignToplevelManagement/RForeignToplevelHandle.h>
#include <CZ/Louvre/Protocols/PresentationTime/RPresentationFeedback.h>
#include <CZ/Louvre/Protocols/PointerConstraints/RLockedPointer.h>
#include <CZ/Louvre/Protocols/PointerConstraints/RConfinedPointer.h>
#include <CZ/Louvre/Protocols/Wayland/GOutput.h>
#include <CZ/Louvre/Private/LSurfacePrivate.h>
#include <CZ/Louvre/Private/LCompositorPrivate.h>
#include <CZ/Louvre/Private/LOutputPrivate.h>
#include <CZ/Louvre/Private/LFactory.h>
#include <CZ/Louvre/Roles/LForeignToplevelController.h>
#include <CZ/Louvre/Roles/LBackgroundBlur.h>
#include <CZ/Louvre/Seat/LSeat.h>
#include <CZ/Louvre/LClient.h>
#include <CZ/Louvre/Seat/LKeyboard.h>
#include <CZ/Louvre/Roles/LSurfaceLock.h>
#include <CZ/Core/CZTime.h>
#include <CZ/Ream/DRM/RDRMTimeline.h>

using namespace CZ::Protocols::Wayland;

LBackgroundBlur *LSurface::backgroundBlur() const noexcept
{
    return imp()->backgroundBlur;
}

LSurface::LSurface(const void *params) noexcept : LFactoryObject(FactoryObjectType), LPRIVATE_INIT_UNIQUE(LSurface)
{
    imp()->pending.damage.reserve(32);
    imp()->pending.bufferDamage.reserve(32);

    compositor()->imp()->surfaces.emplace_back(this);
    imp()->compositorLink = std::prev(compositor()->imp()->surfaces.end());

    compositor()->imp()->layers[LLayerMiddle].emplace_back(this);
    imp()->layerLink = std::prev(compositor()->imp()->layers[LLayerMiddle].end());

    imp()->surfaceResource = ((LSurface::Params*)params)->surfaceResource;
    imp()->backgroundBlur = LFactory::createObject<LBackgroundBlur>(this);
}

LSurface::~LSurface() noexcept
{
    notifyDestruction();
    delete imp()->backgroundBlur.get();

    compositor()->imp()->surfaces.erase(imp()->compositorLink);
    compositor()->imp()->layers[layer()].erase(imp()->layerLink);
}

std::shared_ptr<LSurfaceLock> LSurface::lock() noexcept
{
    return std::shared_ptr<LSurfaceLock>(new LSurfaceLock(this, true));
}

bool LSurface::isLocked() const noexcept
{
    return imp()->pending.lockCount > 0 || !imp()->cached.empty();
}

const std::vector<LSubsurfaceRole *> &LSurface::subsurfacesBelow() const noexcept
{
    return imp()->current.subsurfacesBelow;
}

const std::vector<LSubsurfaceRole *> &LSurface::subsurfacesAbove() const noexcept
{
    return imp()->current.subsurfacesAbove;
}

LSurfaceLayer LSurface::layer() const noexcept
{
    return imp()->layer;
}

void LSurface::raise() noexcept
{
    auto *surf { topmostParent() };

    if (!surf)
        surf = this;

    auto &layerList { compositor()->imp()->layers[surf->layer()] };

    if (layerList.back() == surf)
        return;

    layerList.erase(surf->imp()->layerLink);
    layerList.emplace_back(surf);
    surf->imp()->layerLink = std::prev(layerList.end());
    surf->raised();
}

LSurface *LSurface::parent() const noexcept
{
    return imp()->parent;
}

LSurface *LSurface::topmostParent() const noexcept
{
    auto *p { parent() };

    if (!p)
        return nullptr;

    while (p->parent())
        p = p->parent();

    return p;
}

bool LSurface::isSubchildOf(Role role) const noexcept
{
    return parent() && (parent()->roleId() == role || parent()->isSubchildOf(role));
}

bool LSurface::isSubchildOf(LSurface *surface) const noexcept
{
    if (!surface) return false;

    auto *p { parent() };

    while (p)
    {
        if (p == surface)
            return true;

        p = p->parent();
    }

    return false;
}

LCursorRole *LSurface::cursorRole() const noexcept
{
    if (roleId() == LSurface::Role::Cursor)
        return (LCursorRole*)imp()->role.get();
    else
        return nullptr;
}

LToplevelRole *LSurface::toplevel() const noexcept
{
    if (roleId() == LSurface::Role::Toplevel)
        return (LToplevelRole*)imp()->role.get();
    else
        return nullptr;
}

LPopupRole *LSurface::popup() const noexcept
{
    if (roleId() == LSurface::Role::Popup)
        return (LPopupRole*)imp()->role.get();
    else
        return nullptr;
}

LSubsurfaceRole *LSurface::subsurface() const noexcept
{
    if (roleId() == LSurface::Role::Subsurface)
        return (LSubsurfaceRole*)imp()->role.get();
    else
        return nullptr;
}

LSessionLockRole *LSurface::sessionLock() const noexcept
{
    if (roleId() == LSurface::Role::SessionLock)
        return (LSessionLockRole*)imp()->role.get();
    else
        return nullptr;
}

LLayerRole *LSurface::layerRole() const noexcept
{
    if (roleId() == LSurface::Role::Layer)
        return (LLayerRole*)imp()->role.get();
    else
        return nullptr;
}

LDNDIconRole *LSurface::dndIcon() const noexcept
{
    if (roleId() == LSurface::Role::DNDIcon)
        return (LDNDIconRole*)imp()->role.get();
    else
        return nullptr;
}

LBaseSurfaceRole *LSurface::role() const noexcept
{
    return imp()->role;
}

void LSurface::setPos(SkIPoint newPos) noexcept
{
    imp()->pos = newPos;
}

void LSurface::setPos(Int32 x, Int32 y) noexcept
{
    imp()->pos.fX = x;
    imp()->pos.fY = y;
}

void LSurface::setX(Int32 x) noexcept
{
    imp()->pos.fX = x;
}

void LSurface::setY(Int32 y) noexcept
{
    imp()->pos.fY = y;
}

SkISize LSurface::sizeB() const noexcept
{
    return imp()->sizeB;
}

SkISize LSurface::size() const noexcept
{
    return imp()->size;
}

const SkRegion &LSurface::inputRegion() const noexcept
{
    return imp()->current.inputRegion;
}

const SkRegion &LSurface::opaqueRegion() const noexcept
{
    return imp()->current.opaqueRegion;
}

const SkRegion &LSurface::invisibleRegion() const noexcept
{
    return imp()->current.invisibleRegion;
}

const SkRegion &LSurface::damage() const noexcept
{
    return imp()->current.damage;
}

const SkRegion &LSurface::bufferDamage() const noexcept
{
    return imp()->current.bufferDamage;
}

RContentType LSurface::contentType() const noexcept
{
    return imp()->current.contentType;
}

void LSurface::repaintOutputs() noexcept
{
    for (LOutput *o : outputs())
        o->repaint();
}

bool LSurface::receiveInput() const noexcept
{
    return imp()->stateFlags.has(LSurfacePrivate::ReceiveInput);
}

Int32 LSurface::scale() const noexcept
{
    return imp()->current.scale;
}

bool LSurface::hasPointerFocus() const noexcept
{
    return seat()->pointer()->focus() == this;
}

bool LSurface::hasKeyboardFocus() const noexcept
{
    return seat()->keyboard()->focus() == this;
}

bool LSurface::hasKeyboardGrab() const noexcept
{
    return seat()->keyboard()->grab() == this;
}

std::shared_ptr<RImage> LSurface::image() const noexcept
{
    return imp()->current.image;
}

bool LSurface::hasDamage() const noexcept
{
    return imp()->stateFlags.has(LSurfacePrivate::Damaged);
}

UInt32 LSurface::damageId() const noexcept
{
    return imp()->damageId;
}

const SkRect &LSurface::srcRect() const noexcept
{
    return imp()->srcRect;
}

CZTransform LSurface::bufferTransform() const noexcept
{
    return imp()->current.transform;
}

LSurface::PointerConstraintMode LSurface::pointerConstraintMode() const noexcept
{
    return imp()->current.pointerConstraintMode;
}

const SkRegion &LSurface::pointerConstraintRegion() const noexcept
{
    return imp()->current.pointerConstraintRegion;
}

void LSurface::enablePointerConstraint(bool enabled)
{
    if (enabled && !hasPointerFocus())
        return;

    switch (pointerConstraintMode())
    {
    case PointerConstraintMode::Lock:
    {
        if (!imp()->current.lockedPointerRes)
            break;

        if (enabled)
            imp()->current.lockedPointerRes->locked();
        else
            imp()->current.lockedPointerRes->unlocked();
        break;
    }
    case PointerConstraintMode::Confine:
    {
        if (!imp()->current.confinedPointerRes)
            break;

        if (enabled)
            imp()->current.confinedPointerRes->confined();
        else
            imp()->current.confinedPointerRes->unconfined();
        break;
    }
    default:
        break;
    }
}

bool LSurface::pointerConstraintEnabled() const noexcept
{
    switch (pointerConstraintMode())
    {
    case PointerConstraintMode::Lock:
    {
        if (!imp()->current.lockedPointerRes)
            break;

        return imp()->current.lockedPointerRes->constrained();
    }
    case PointerConstraintMode::Confine:
    {
        if (!imp()->current.confinedPointerRes)
            break;

        return imp()->current.confinedPointerRes->constrained();
    }
    default:
        break;
    }

    return false;
}

SkPoint LSurface::lockedPointerPosHint() const noexcept
{
    return imp()->current.lockedPointerPosHint;
}

LSurface::Role LSurface::roleId() const noexcept
{
    if (role())
        return (LSurface::Role)role()->roleId();
    else
        return Undefined;
}

SkIPoint LSurface::pos() const noexcept
{
    return imp()->pos;
}

SkIPoint LSurface::rolePos() const
{
    if (role())
        return role()->rolePos();

    return imp()->pos;
}

void LSurface::sendOutputEnterEvent(LOutput *output) noexcept
{
    if (!output || imp()->stateFlags.has(LSurfacePrivate::Destroyed) || imp()->outputs.contains(output))
        return;

    imp()->outputs.emplace(output);

    for (GOutput *global : client()->outputGlobals())
        if (global->output() == output)
            surfaceResource()->enter(global);

    imp()->sendPreferredScale();

    if (toplevel())
    {
        for (auto *controller : toplevel()->foreignControllers())
        {
            controller->resource().outputEnter(output);
            controller->resource().done();
        }
    }
}

void LSurface::sendOutputLeaveEvent(LOutput *output) noexcept
{
    if (imp()->stateFlags.has(LSurfacePrivate::Destroyed) ||
        !output ||
        !imp()->outputs.contains(output) ||
        (role() && role()->exclusiveOutput() == output && output->state() == LOutput::Initialized))
        return;

    imp()->outputs.erase(output);

    for (GOutput *global : client()->outputGlobals())
        if (global->output() == output)
            surfaceResource()->leave(global);

    imp()->sendPreferredScale();

    if (toplevel())
    {
        for (auto *controller : toplevel()->foreignControllers())
        {
            controller->resource().outputLeave(output);
            controller->resource().done();
        }
    }

    return;
}

const std::unordered_set<LOutput*> &LSurface::outputs() const noexcept
{
    return imp()->outputs;
}

void LSurface::requestNextFrame(bool clearDamage) noexcept
{
    if (imp()->stateFlags.has(LSurfacePrivate::Destroyed))
        return;

    if (clearDamage)
    {
        // Mark feedback res as "pending to be presented" on this output
        // the final result is sent once the backend notifies the page flip/discard
        if (auto *output = compositor()->imp()->currentOutput)
        {
            auto &feedbackList { imp()->current.presentationFeedbackRes };

            while (!feedbackList.empty())
            {
                if (!feedbackList.front())
                {
                    feedbackList.pop_front();
                    continue;
                }

                feedbackList.front()->output.reset(output);
                feedbackList.front()->paintEventId = output->backend()->paintEventId();
                // Move res to the end of waitingPresentationFeedback
                output->imp()->waitingPresentationFeedback.splice(
                    output->imp()->waitingPresentationFeedback.end(),
                    feedbackList,
                    feedbackList.begin());
            }
        }

        imp()->current.bufferDamage.setEmpty();
        imp()->current.damage.setEmpty();
        imp()->stateFlags.remove(LSurfacePrivate::Damaged);
    }

    imp()->current.frames.sendDoneAndDestroyFrames();
}

bool LSurface::mapped() const noexcept
{
    return imp()->stateFlags.has(LSurfacePrivate::Mapped);
}

bool LSurface::prefersVSync() noexcept
{
    return imp()->stateFlags.has(LSurfacePrivate::VSync);
}

Wayland::RWlSurface *LSurface::surfaceResource() const noexcept
{
    return imp()->surfaceResource;
}

std::vector<IdleInhibit::RIdleInhibitor *> LSurface::idleInhibitorResources() const noexcept
{
    return imp()->idleInhibitorResources;
}

wl_buffer *LSurface::bufferResource() const noexcept
{
    return (wl_buffer*)imp()->current.buffer.buffer.res();
}

LClient *LSurface::client() const noexcept
{
    return surfaceResource()->client();
}

bool LSurfaceBuffer::release() noexcept
{
    if (buffer.res())
    {
        if (!released)
        {
            released = true;
            wl_buffer_send_release(buffer.res());
        }
    }
    else
        released = true;

    if (releaseTimeline)
    {
        if (!signaled)
        {
            auto image { weakImage.lock() };

            if (image && image->readSync())
            {
                signaled = true;
                auto fd { image->readSync()->fd() };

                if (fd.get() >= 0)
                    releaseTimeline->importSyncFile(fd.release(), releasePoint, CZOwn::Own);
                else
                    releaseTimeline->signalPoint(releasePoint);
            }
            else
            {
                signaled = true;
                releaseTimeline->signalPoint(releasePoint);
            }
        }
    }
    else
        signaled = true;

    if (released && signaled)
        return true;
    else if (!queued)
    {
        queued = true;
        compositor()->imp()->unreleasedBuffers.emplace_back(*this);
        compositor()->imp()->unlockPoll();
    }

    return false;
}
