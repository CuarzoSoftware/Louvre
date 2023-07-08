#include <protocols/LinuxDMABuf/private/LDMABufferPrivate.h>
#include <protocols/Wayland/RCallback.h>
#include <protocols/Wayland/GOutput.h>
#include <private/LSurfacePrivate.h>
#include <private/LCompositorPrivate.h>
#include <private/LOutputPrivate.h>
#include <private/LTexturePrivate.h>
#include <LTime.h>

using namespace Louvre::Protocols::Wayland;

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

LSurface::LSurface(LSurface::Params *params, GLuint textureUnit)
{
    m_imp = new LSurfacePrivate();
    imp()->texture = new LTexture(textureUnit);
    imp()->textureBackup = imp()->texture;
    imp()->surfaceResource = params->surfaceResource;
}

LSurface::~LSurface()
{
    if (imp()->texture && imp()->texture != imp()->textureBackup && imp()->texture->imp()->pendingDelete)
        delete imp()->texture;

    delete imp()->textureBackup;
    delete m_imp;
}

void LSurface::setPosC(const LPoint &newPos)
{
    imp()->posC = newPos;
}

void LSurface::setPosC(Int32 x, Int32 y)
{
    imp()->posC.setX(x);
    imp()->posC.setY(y);
}

void LSurface::setXC(Int32 x)
{
    imp()->posC.setX(x);
}

void LSurface::setYC(Int32 y)
{
    imp()->posC.setY(y);
}

const LSize &LSurface::sizeB() const
{
    return imp()->currentSizeB;
}

const LSize &LSurface::sizeS() const
{
    return imp()->currentSizeS;
}

const LSize &LSurface::sizeC() const
{
    return imp()->currentSizeC;
}

const LRegion &LSurface::inputRegionS() const
{
    return imp()->currentInputRegionS;
}

const LRegion &LSurface::inputRegionC() const
{
    return imp()->currentInputRegionC;
}

const LRegion &LSurface::opaqueRegionS() const
{
    return imp()->currentOpaqueRegionS;
}

const LRegion &LSurface::opaqueRegionC() const
{
    return imp()->currentOpaqueRegionC;
}

const LRegion &LSurface::translucentRegionS() const
{
    return imp()->currentTranslucentRegionS;
}

const LRegion &LSurface::translucentRegionC() const
{
    return imp()->currentTranslucentRegionC;
}

const LRegion &LSurface::damagesB() const
{
    return imp()->currentDamagesB;
}

const LRegion &LSurface::damagesC() const
{
    return imp()->currentDamagesC;
}

void LSurface::setMinimized(bool state)
{
    if (state != minimized())
    {
        imp()->minimized = state;
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
    return imp()->receiveInput;
}

Int32 LSurface::bufferScale() const
{
    return imp()->current.bufferScale;
}

LTexture *LSurface::texture() const
{
    return imp()->texture;
}

bool LSurface::hasDamage() const
{
    return imp()->damaged;
}

UInt32 LSurface::damageId() const
{
    return imp()->damageId;
}

bool LSurface::minimized() const
{
    return imp()->minimized;
}

LSurface::Role LSurface::roleId() const
{
    if (role())
        return (LSurface::Role)role()->roleId();
    else
        return Undefined;
}

const LPoint &LSurface::posC() const
{
    return imp()->posC;
}

const LPoint &LSurface::rolePosC() const
{
    if (role())
        return role()->rolePosC();

    return imp()->posC;
}

void LSurface::sendOutputEnterEvent(LOutput *output)
{
    if (!output)
        return;

    // Check if already sent
    for (LOutput *o : imp()->outputs)
        if (o == output)
            return;

    imp()->outputs.push_back(output);

    for (GOutput *g : client()->outputGlobals())
    {
        if (g->output() == output)
        {
            surfaceResource()->enter(g);
            imp()->sendPreferredScale();
            return;
        }
    }
}

void LSurface::sendOutputLeaveEvent(LOutput *output)
{
    if (!output)
        return;

    for (list<LOutput*>::iterator o = imp()->outputs.begin(); o != imp()->outputs.end(); o++)
    {
        if (*o == output)
        {
            imp()->outputs.erase(o);
            for (GOutput *g : client()->outputGlobals())
            {
                if (g->output() == output)
                {
                    surfaceResource()->leave(g);
                    imp()->sendPreferredScale();
                    return;
                }
            }
            return;
        }
    }
}

const list<LOutput *> &LSurface::outputs() const
{
    return imp()->outputs;
}

void LSurface::requestNextFrame(bool clearDamage)
{
    if (clearDamage)
    {
        imp()->currentDamagesB.clear();
        imp()->currentDamagesC.clear();
        imp()->damaged = false;
    }

    UInt32 ms = LTime::ms();

    while (!imp()->frameCallbacks.empty())
    {
        Wayland::RCallback *rCallback = imp()->frameCallbacks.front();
        if (rCallback->commited)
        {
            rCallback->done(ms);
            rCallback->destroy();
        }
        else
            break;
    }

    client()->flush();
}

bool LSurface::mapped() const
{
    return imp()->mapped;
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

LSurface *findTopmostParent(LSurface *surface)
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

Louvre::LSurface *LSurface::prevSurface() const
{
    if (this == compositor()->surfaces().front())
        return nullptr;
    else
        return *std::prev(imp()->compositorLink);
}

Louvre::LSurface *LSurface::nextSurface() const
{
    if (this == compositor()->surfaces().back())
        return nullptr;
    else
        return *std::next(imp()->compositorLink);
}
