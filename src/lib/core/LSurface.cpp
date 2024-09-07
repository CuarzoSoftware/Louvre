#include <protocols/ForeignToplevelManagement/RForeignToplevelHandle.h>
#include <protocols/PresentationTime/RPresentationFeedback.h>
#include <protocols/PointerConstraints/RLockedPointer.h>
#include <protocols/PointerConstraints/RConfinedPointer.h>
#include <protocols/Wayland/RCallback.h>
#include <protocols/Wayland/GOutput.h>
#include <private/LSurfacePrivate.h>
#include <private/LCompositorPrivate.h>
#include <private/LOutputPrivate.h>
#include <private/LTexturePrivate.h>
#include <LForeignToplevelController.h>
#include <LTime.h>
#include <LSeat.h>
#include <LClient.h>
#include <LKeyboard.h>

using namespace Louvre::Protocols::Wayland;

LSurface::LSurface(const void *params) noexcept : LFactoryObject(FactoryObjectType), LPRIVATE_INIT_UNIQUE(LSurface)
{
    imp()->pendingDamage.reserve(LOUVRE_MAX_DAMAGE_RECTS);
    imp()->pendingDamageB.reserve(LOUVRE_MAX_DAMAGE_RECTS);
    imp()->texture = new LTexture(true);
    imp()->textureBackup = imp()->texture;
    imp()->surfaceResource = ((LSurface::Params*)params)->surfaceResource;
    imp()->current.onBufferDestroyListener.notify = [](wl_listener *listener, void *)
    {
        LSurfacePrivate::State *state { (LSurfacePrivate::State *)listener };
        state->bufferRes = nullptr;
    };
    imp()->pending.onBufferDestroyListener.notify = imp()->current.onBufferDestroyListener.notify;
}

LSurface::~LSurface()
{
    if (imp()->pending.bufferRes)
        wl_list_remove(&imp()->pending.onBufferDestroyListener.link);

    if (imp()->current.bufferRes)
        wl_list_remove(&imp()->current.onBufferDestroyListener.link);

    imp()->lastPointerEventView = nullptr;

    if (imp()->texture && imp()->texture != imp()->textureBackup && imp()->texture->m_pendingDelete)
        delete imp()->texture;

    delete imp()->textureBackup;
}

LCursorRole *LSurface::cursorRole() const noexcept
{
    if (roleId() == LSurface::Role::Cursor)
        return (LCursorRole*)imp()->current.role;
    else
        return nullptr;
}

LToplevelRole *LSurface::toplevel() const noexcept
{
    if (roleId() == LSurface::Role::Toplevel)
        return (LToplevelRole*)imp()->current.role;
    else
        return nullptr;
}

LPopupRole *LSurface::popup() const noexcept
{
    if (roleId() == LSurface::Role::Popup)
        return (LPopupRole*)imp()->current.role;
    else
        return nullptr;
}

LSubsurfaceRole *LSurface::subsurface() const noexcept
{
    if (roleId() == LSurface::Role::Subsurface)
        return (LSubsurfaceRole*)imp()->current.role;
    else
        return nullptr;
}

LSessionLockRole *LSurface::sessionLock() const noexcept
{
    if (roleId() == LSurface::Role::SessionLock)
        return (LSessionLockRole*)imp()->current.role;
    else
        return nullptr;
}

LLayerRole *LSurface::layerRole() const noexcept
{
    if (roleId() == LSurface::Role::Layer)
        return (LLayerRole*)imp()->current.role;
    else
        return nullptr;
}

LXWindowRole *LSurface::xWindowRole() const noexcept
{
    if (roleId() == LSurface::Role::XWindow)
        return (LXWindowRole*)imp()->current.role;
    else
        return nullptr;
}

LDNDIconRole *LSurface::dndIcon() const noexcept
{
    if (roleId() == LSurface::Role::DNDIcon)
        return (LDNDIconRole*)imp()->current.role;
    else
        return nullptr;
}

LBaseSurfaceRole *LSurface::role() const noexcept
{
    return imp()->current.role;
}

void LSurface::setPos(const LPoint &newPos) noexcept
{
    imp()->pos = newPos;
}

void LSurface::setPos(Int32 x, Int32 y) noexcept
{
    imp()->pos.setX(x);
    imp()->pos.setY(y);
}

void LSurface::setX(Int32 x) noexcept
{
    imp()->pos.setX(x);
}

void LSurface::setY(Int32 y) noexcept
{
    imp()->pos.setY(y);
}

const LSize &LSurface::sizeB() const noexcept
{
    return imp()->sizeB;
}

const LSize &LSurface::size() const noexcept
{
    return imp()->size;
}

const LRegion &LSurface::inputRegion() const noexcept
{
    return imp()->currentInputRegion;
}

const LRegion &LSurface::opaqueRegion() const noexcept
{
    return imp()->currentOpaqueRegion;
}

const LRegion &LSurface::translucentRegion() const noexcept
{
    return imp()->currentTranslucentRegion;
}

const LRegion &LSurface::damageB() const noexcept
{
    return imp()->currentDamageB;
}

LContentType LSurface::contentType() const noexcept
{
    return imp()->current.contentType;
}

const LRegion &LSurface::damage() const noexcept
{
    return imp()->currentDamage;
}

void LSurface::setMinimized(bool state)
{
    if (state != minimized())
    {
        imp()->stateFlags.setFlag(LSurfacePrivate::Minimized, state);

        if (toplevel())
        {
            for (auto *controller : toplevel()->foreignControllers())
            {
                controller->resource().updateState();
                controller->resource().done();
            }
        }

        minimizedChanged();

        for (LSurface *child : children())
            child->setMinimized(state);
    }
}

void LSurface::repaintOutputs() noexcept
{
    for (LOutput *o : outputs())
        o->repaint();
}

bool LSurface::receiveInput() const noexcept
{
    return imp()->stateFlags.check(LSurfacePrivate::ReceiveInput);
}

Int32 LSurface::bufferScale() const noexcept
{
    return imp()->current.bufferScale;
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

LTexture *LSurface::texture() const noexcept
{
    return imp()->texture;
}

bool LSurface::hasDamage() const noexcept
{
    return imp()->stateFlags.check(LSurfacePrivate::Damaged);
}

UInt32 LSurface::damageId() const noexcept
{
    return imp()->damageId;
}

bool LSurface::minimized() const noexcept
{
    return imp()->stateFlags.check(LSurfacePrivate::Minimized);
}

const LRectF &LSurface::srcRect() const noexcept
{
    return imp()->srcRect;
}

LTransform LSurface::bufferTransform() const noexcept
{
    return imp()->current.transform;
}

LSurface::PointerConstraintMode LSurface::pointerConstraintMode() const noexcept
{
    if (imp()->lockedPointerRes)
        return PointerConstraintMode::Lock;
    else if (imp()->confinedPointerRes)
        return PointerConstraintMode::Confine;

    return PointerConstraintMode::Free;
}

const LRegion &LSurface::pointerConstraintRegion() const noexcept
{
    return imp()->pointerConstraintRegion;
}

void LSurface::enablePointerConstraint(bool enabled)
{
    if (enabled && !hasPointerFocus())
        return;

    if (imp()->lockedPointerRes)
    {
        if (enabled)
            imp()->lockedPointerRes->locked();
        else
            imp()->lockedPointerRes->unlocked();
    }
    else if (imp()->confinedPointerRes)
    {
        if (enabled)
            imp()->confinedPointerRes->confined();
        else
            imp()->confinedPointerRes->unconfined();
    }
}

bool LSurface::pointerConstraintEnabled() const noexcept
{
    if (imp()->lockedPointerRes)
        return imp()->lockedPointerRes->constrained();
    else if (imp()->confinedPointerRes)
        return imp()->confinedPointerRes->constrained();

    return false;
}

const LPointF &LSurface::lockedPointerPosHint() const noexcept
{
    return imp()->current.lockedPointerPosHint;
}

LSurfaceLayer LSurface::layer() const noexcept
{
    return imp()->layer;
}

LSurface::Role LSurface::roleId() const noexcept
{
    if (role())
        return (LSurface::Role)role()->roleId();
    else
        return Undefined;
}

const LPoint &LSurface::pos() const noexcept
{
    return imp()->pos;
}

const LPoint &LSurface::rolePos() const
{
    if (role())
        return role()->rolePos();

    return imp()->pos;
}

void LSurface::sendOutputEnterEvent(LOutput *output) noexcept
{
    if (imp()->stateFlags.check(LSurfacePrivate::Destroyed))
        return;

    if (!output)
        return;

    // Check if already sent
    for (LOutput *o : imp()->outputs)
        if (o == output)
            return;

    imp()->outputs.push_back(output);

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
    if (imp()->stateFlags.check(LSurfacePrivate::Destroyed))
        return;

    if (!output || (role() && role()->exclusiveOutput() == output && output->state() == LOutput::Initialized))
        return;

    for (std::size_t i = 0; i < imp()->outputs.size(); i++)
    {
        if (imp()->outputs[i] == output)
        {
            imp()->outputs[i] = std::move(imp()->outputs.back());
            imp()->outputs.pop_back();

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
    }
}

const std::vector<LOutput *> &LSurface::outputs() const noexcept
{
    return imp()->outputs;
}

void LSurface::requestNextFrame(bool clearDamage) noexcept
{
    if (imp()->stateFlags.check(LSurfacePrivate::Destroyed))
        return;

    for (auto *presentation : imp()->presentationFeedbackResources)
    {
        if (presentation->m_commitId >= 0 && !presentation->m_output)
        {
            if (presentation->m_commitId == imp()->commitId && compositor()->imp()->currentOutput)
            {
                presentation->m_outputSet = true;
                presentation->m_output.reset(compositor()->imp()->currentOutput);
            }
            else
                presentation->m_commitId = -2;
        }
    }

    if (clearDamage)
    {
        imp()->currentDamageB.clear();
        imp()->currentDamage.clear();
        imp()->stateFlags.remove(LSurfacePrivate::Damaged);
    }

    while (!imp()->frameCallbacks.empty())
    {
        if (!imp()->frameCallbacks.front()->m_commited)
            break;

        imp()->frameCallbacks.front()->done(LTime::ms());
        imp()->frameCallbacks.front()->destroy();
    }
}

bool LSurface::mapped() const noexcept
{
    return imp()->stateFlags.check(LSurfacePrivate::Mapped);
}

bool LSurface::preferVSync() noexcept
{
    return imp()->stateFlags.check(LSurfacePrivate::VSync);
}

const std::vector<LSurfaceView *> &LSurface::views() const noexcept
{
    return imp()->views;
}

Wayland::RSurface *LSurface::surfaceResource() const noexcept
{
    return imp()->surfaceResource;
}

std::vector<IdleInhibit::RIdleInhibitor *> LSurface::idleInhibitorResources() const noexcept
{
    return imp()->idleInhibitorResources;
}

wl_buffer *LSurface::bufferResource() const noexcept
{
    return (wl_buffer*)imp()->current.bufferRes;
}

bool LSurface::hasBuffer() const noexcept
{
    return imp()->current.hasBuffer;
}

LClient *LSurface::client() const noexcept
{
    return surfaceResource()->client();
}

LSurface *LSurface::parent() const noexcept
{
    return imp()->parent;
}

static LSurface *findTopmostParent(LSurface *surface) noexcept
{
    if (surface->parent() == nullptr)
        return surface;

    return findTopmostParent(surface->parent());
}

LSurface *LSurface::topmostParent() const noexcept
{
    if (parent() == nullptr)
        return nullptr;

    return findTopmostParent(parent());
}

const std::list<LSurface *> &LSurface::children() const noexcept
{
    return imp()->children;
}

static bool isChildOfPopup(const LSurface *surface) noexcept
{
    if (surface->parent())
    {
        if (surface->parent()->popup())
            return true;

        return isChildOfPopup(surface->parent());
    }
    return false;
}

bool LSurface::isPopupSubchild() const noexcept
{
    return isChildOfPopup(this);
}

static bool hasPopupChildren(const LSurface *surface) noexcept
{
    for (LSurface *c : surface->children())
    {
        if (c->popup())
            return true;

        if (hasPopupChildren(c))
            return true;
    }

    return false;
}

bool LSurface::hasPopupSubchild() const noexcept
{
    return hasPopupChildren(this);
}

bool LSurface::isSubchildOf(LSurface *parent) const noexcept
{
    if (!parent)
        return false;

    for (LSurface *c : parent->children())
    {
        if (c == this)
            return true;

        if (isSubchildOf(c))
            return true;
    }

    return false;
}

void LSurface::raise()
{
    if (compositor()->imp()->surfaceRaiseAllowedCounter > 0 || imp()->stateFlags.check(LSurfacePrivate::Destroyed))
        return;

    if (parent() && (subsurface() || toplevel() || (imp()->pending.role && (imp()->pending.role->roleId() == Role::Subsurface || imp()->pending.role->roleId() == Role::Toplevel))))
        parent()->raise();
    else
        imp()->setLayer(layer());
}

LSurface *LSurface::prevSurface() const noexcept
{
    if (imp()->stateFlags.check(LSurfacePrivate::Destroyed) || this == compositor()->surfaces().front())
        return nullptr;
    else
        return *std::prev(imp()->compositorLink);
}

LSurface *LSurface::nextSurface() const noexcept
{
    if (imp()->stateFlags.check(LSurfacePrivate::Destroyed) || this == compositor()->surfaces().back())
        return nullptr;
    else
        return *std::next(imp()->compositorLink);
}
