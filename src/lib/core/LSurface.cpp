#include <protocols/PresentationTime/private/RPresentationFeedbackPrivate.h>
#include <protocols/LinuxDMABuf/private/LDMABufferPrivate.h>
#include <protocols/Wayland/RCallback.h>
#include <protocols/Wayland/GOutput.h>
#include <private/LSurfacePrivate.h>
#include <private/LCompositorPrivate.h>
#include <private/LOutputPrivate.h>
#include <private/LTexturePrivate.h>
#include <LTime.h>
#include <LSeat.h>
#include <LClient.h>
#include <LKeyboard.h>

using namespace Louvre::Protocols::Wayland;

LSurface::LSurface(const void *params) : LPRIVATE_INIT_UNIQUE(LSurface)
{
    imp()->texture = new LTexture();
    imp()->textureBackup = imp()->texture;
    imp()->surfaceResource = ((LSurface::Params*)params)->surfaceResource;
}

LSurface::~LSurface()
{
    imp()->lastPointerEventView = nullptr;

    if (imp()->texture && imp()->texture != imp()->textureBackup && imp()->texture->imp()->pendingDelete)
        delete imp()->texture;

    delete imp()->textureBackup;
}

LCursorRole *LSurface::cursorRole() const
{
    if (roleId() == LSurface::Role::Cursor)
        return (LCursorRole*)imp()->current.role;
    else
        return nullptr;
}

LToplevelRole *LSurface::toplevel() const
{
    if (roleId() == LSurface::Role::Toplevel)
        return (LToplevelRole*)imp()->current.role;
    else
        return nullptr;
}

LPopupRole *LSurface::popup() const
{
    if (roleId() == LSurface::Role::Popup)
        return (LPopupRole*)imp()->current.role;
    else
        return nullptr;
}

LSubsurfaceRole *LSurface::subsurface() const
{
    if (roleId() == LSurface::Role::Subsurface)
        return (LSubsurfaceRole*)imp()->current.role;
    else
        return nullptr;
}

LDNDIconRole *LSurface::dndIcon() const
{
    if (roleId() == LSurface::Role::DNDIcon)
        return (LDNDIconRole*)imp()->current.role;
    else
        return nullptr;
}

LBaseSurfaceRole *LSurface::role() const
{
    return imp()->current.role;
}

void LSurface::setPos(const LPoint &newPos)
{
    imp()->pos = newPos;
}

void LSurface::setPos(Int32 x, Int32 y)
{
    imp()->pos.setX(x);
    imp()->pos.setY(y);
}

void LSurface::setX(Int32 x)
{
    imp()->pos.setX(x);
}

void LSurface::setY(Int32 y)
{
    imp()->pos.setY(y);
}

const LSize &LSurface::sizeB() const
{
    return imp()->sizeB;
}

const LSize &LSurface::size() const
{
    return imp()->size;
}

const LRegion &LSurface::inputRegion() const
{
    return imp()->currentInputRegion;
}

const LRegion &LSurface::opaqueRegion() const
{
    return imp()->currentOpaqueRegion;
}

const LRegion &LSurface::translucentRegion() const
{
    return imp()->currentTranslucentRegion;
}

const LRegion &LSurface::damageB() const
{
    return imp()->currentDamageB;
}

const LRegion &LSurface::damage() const
{
    return imp()->currentDamage;
}

void LSurface::setMinimized(bool state)
{
    if (state != minimized())
    {
        imp()->stateFlags.setFlag(LSurfacePrivate::Minimized, state);
        minimizedChanged();

        for (LSurface *child : children())
            child->setMinimized(state);
    }
}

void LSurface::repaintOutputs()
{
    for (LOutput *o : outputs())
        o->repaint();
}

bool LSurface::receiveInput() const
{
    return imp()->stateFlags.check(LSurfacePrivate::ReceiveInput);
}

Int32 LSurface::bufferScale() const
{
    return imp()->current.bufferScale;
}

bool LSurface::hasPointerFocus() const
{
    return seat()->pointer()->focus() == this;
}

bool LSurface::hasKeyboardFocus() const
{
    return seat()->keyboard()->focus() == this;
}

LTexture *LSurface::texture() const
{
    return imp()->texture;
}

bool LSurface::hasDamage() const
{
    return imp()->stateFlags.check(LSurfacePrivate::Damaged);
}

UInt32 LSurface::damageId() const
{
    return imp()->damageId;
}

bool LSurface::minimized() const
{
    return imp()->stateFlags.check(LSurfacePrivate::Minimized);
}

const LRectF &LSurface::srcRect() const
{
    return imp()->srcRect;
}

LFramebuffer::Transform LSurface::bufferTransform() const
{
    return imp()->current.transform;
}

LSurface::Role LSurface::roleId() const
{
    if (role())
        return (LSurface::Role)role()->roleId();
    else
        return Undefined;
}

const LPoint &LSurface::pos() const
{
    return imp()->pos;
}

const LPoint &LSurface::rolePos() const
{
    if (role())
        return role()->rolePos();

    return imp()->pos;
}

void LSurface::sendOutputEnterEvent(LOutput *output)
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
}

void LSurface::sendOutputLeaveEvent(LOutput *output)
{
    if (imp()->stateFlags.check(LSurfacePrivate::Destroyed))
        return;

    if (!output)
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
            return;
        }
    }
}

const std::vector<LOutput *> &LSurface::outputs() const
{
    return imp()->outputs;
}

void LSurface::requestNextFrame(bool clearDamage)
{
    if (imp()->stateFlags.check(LSurfacePrivate::Destroyed))
        return;

    for (auto *presentation : imp()->presentationFeedbackResources)
    {
        if (presentation->imp()->commitId >= 0 && !presentation->imp()->output.get())
        {
            if (presentation->imp()->commitId == imp()->commitId && compositor()->imp()->currentOutput)
            {
                presentation->imp()->outputSet = true;
                presentation->imp()->output.reset(compositor()->imp()->currentOutput);
            }
            else
                presentation->imp()->commitId = -2;
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
        if (!imp()->frameCallbacks.front()->commited)
            break;

        imp()->frameCallbacks.front()->done(LTime::ms());
        imp()->frameCallbacks.front()->destroy();
    }
}

bool LSurface::mapped() const
{
    return imp()->stateFlags.check(LSurfacePrivate::Mapped);
}

bool LSurface::preferVSync()
{
    return imp()->stateFlags.check(LSurfacePrivate::VSync);
}

const std::vector<LSurfaceView *> &LSurface::views() const
{
    return imp()->views;
}

Wayland::RSurface *LSurface::surfaceResource() const
{
    return imp()->surfaceResource;
}

wl_buffer *LSurface::buffer() const
{
    return (wl_buffer*)imp()->current.buffer;
}

LClient *LSurface::client() const
{
    return surfaceResource()->client();
}

LSurface *LSurface::parent() const
{
    return imp()->parent;
}

static LSurface *findTopmostParent(LSurface *surface)
{
    if (surface->parent() == nullptr)
        return surface;

    return findTopmostParent(surface->parent());
}

LSurface *LSurface::topmostParent() const
{
    if (parent() == nullptr)
        return nullptr;

    return findTopmostParent(parent());
}

const list<LSurface *> &LSurface::children() const
{
    return imp()->children;
}

static bool isChildOfPopup(const LSurface *surface)
{
    if (surface->parent())
    {
        if (surface->parent()->popup())
            return true;

        return isChildOfPopup(surface->parent());
    }
    return false;
}

bool LSurface::isPopupSubchild() const
{
    return isChildOfPopup(this);
}

static bool hasPopupChildren(const LSurface *surface)
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

bool LSurface::hasPopupSubchild() const
{
    return hasPopupChildren(this);
}

bool LSurface::isSubchildOf(LSurface *parent) const
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
    if (imp()->stateFlags.check(LSurfacePrivate::Destroyed))
        return;

    if (parent())
    {
        parent()->raise();
        return;
    }

    compositor()->imp()->raiseChildren(this);
}

LSurface *LSurface::prevSurface() const
{
    if (imp()->stateFlags.check(LSurfacePrivate::Destroyed) || this == compositor()->surfaces().front())
        return nullptr;
    else
        return *std::prev(imp()->compositorLink);
}

LSurface *LSurface::nextSurface() const
{
    if (imp()->stateFlags.check(LSurfacePrivate::Destroyed) || this == compositor()->surfaces().back())
        return nullptr;
    else
        return *std::next(imp()->compositorLink);
}
