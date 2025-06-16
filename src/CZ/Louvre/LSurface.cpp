#include <CZ/Louvre/Protocols/ForeignToplevelManagement/RForeignToplevelHandle.h>
#include <CZ/Louvre/Protocols/PresentationTime/RPresentationFeedback.h>
#include <CZ/Louvre/Protocols/PointerConstraints/RLockedPointer.h>
#include <CZ/Louvre/Protocols/PointerConstraints/RConfinedPointer.h>
#include <CZ/Louvre/Protocols/Wayland/RCallback.h>
#include <CZ/Louvre/Protocols/Wayland/GOutput.h>
#include <CZ/Louvre/Private/LSurfacePrivate.h>
#include <CZ/Louvre/Private/LCompositorPrivate.h>
#include <CZ/Louvre/Private/LOutputPrivate.h>
#include <CZ/Louvre/Private/LTexturePrivate.h>
#include <CZ/Louvre/Private/LFactory.h>
#include <LForeignToplevelController.h>
#include <LBackgroundBlur.h>
#include <LTime.h>
#include <LSeat.h>
#include <LClient.h>
#include <LKeyboard.h>

using namespace Louvre::Protocols::Wayland;

LBackgroundBlur *LSurface::backgroundBlur() const noexcept
{
    return imp()->backgroundBlur;
}

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
    imp()->backgroundBlur = LFactory::createObject<LBackgroundBlur>(this);
}

LSurface::~LSurface()
{
    notifyDestruction();

    delete imp()->backgroundBlur.get();

    if (imp()->pending.bufferRes)
        wl_list_remove(&imp()->pending.onBufferDestroyListener.link);

    if (imp()->current.bufferRes)
        wl_list_remove(&imp()->current.onBufferDestroyListener.link);

    if (imp()->texture && imp()->texture != imp()->textureBackup && imp()->texture->m_pendingDelete)
        delete imp()->texture;

    delete imp()->textureBackup;
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
    return imp()->currentInputRegion;
}

const SkRegion &LSurface::opaqueRegion() const noexcept
{
    return imp()->currentOpaqueRegion;
}

const SkRegion &LSurface::translucentRegion() const noexcept
{
    return imp()->currentTranslucentRegion;
}

const SkRegion &LSurface::invisibleRegion() const noexcept
{
    return imp()->currentInvisibleRegion;
}

const SkRegion &LSurface::damageB() const noexcept
{
    return imp()->currentDamageB;
}

LContentType LSurface::contentType() const noexcept
{
    return imp()->current.contentType;
}

const SkRegion &LSurface::damage() const noexcept
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
    return imp()->stateFlags.has(LSurfacePrivate::ReceiveInput);
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
    return imp()->stateFlags.has(LSurfacePrivate::Damaged);
}

UInt32 LSurface::damageId() const noexcept
{
    return imp()->damageId;
}

bool LSurface::minimized() const noexcept
{
    return imp()->stateFlags.has(LSurfacePrivate::Minimized);
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
    if (imp()->lockedPointerRes)
        return PointerConstraintMode::Lock;
    else if (imp()->confinedPointerRes)
        return PointerConstraintMode::Confine;

    return PointerConstraintMode::Free;
}

const SkRegion &LSurface::pointerConstraintRegion() const noexcept
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

const SkPoint &LSurface::lockedPointerPosHint() const noexcept
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
    if (imp()->stateFlags.has(LSurfacePrivate::Destroyed))
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
    if (imp()->stateFlags.has(LSurfacePrivate::Destroyed))
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
    if (imp()->stateFlags.has(LSurfacePrivate::Destroyed))
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
        imp()->currentDamageB.setEmpty();
        imp()->currentDamage.setEmpty();
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
    return imp()->stateFlags.has(LSurfacePrivate::Mapped);
}

bool LSurface::preferVSync() noexcept
{
    return imp()->stateFlags.has(LSurfacePrivate::VSync);
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
    if (compositor()->imp()->surfaceRaiseAllowedCounter > 0 || imp()->stateFlags.has(LSurfacePrivate::Destroyed))
        return;

    if (parent() && (subsurface() || toplevel()))
        parent()->raise();
    else
        imp()->setLayer(layer());
}

LSurface *LSurface::prevSurface() const noexcept
{
    if (imp()->stateFlags.has(LSurfacePrivate::Destroyed) || this == compositor()->surfaces().front())
        return nullptr;
    else
        return *std::prev(imp()->compositorLink);
}

LSurface *LSurface::nextSurface() const noexcept
{
    if (imp()->stateFlags.has(LSurfacePrivate::Destroyed) || this == compositor()->surfaces().back())
        return nullptr;
    else
        return *std::next(imp()->compositorLink);
}
