#include "LOutputMode.h"
#include "drm_fourcc.h"

#include <private/LSurfacePrivate.h>
#include <private/LCompositorPrivate.h>
#include <private/LOutputPrivate.h>
#include <private/LTexturePrivate.h>

#include <protocols/LinuxDMABuf/private/LDMABufferPrivate.h>

#include <protocols/WpPresentationTime/private/RWpPresentationFeedbackPrivate.h>
#include <protocols/WpPresentationTime/presentation-time.h>

#include <protocols/XdgShell/xdg-shell.h>
#include <protocols/Wayland/GOutput.h>
#include <protocols/Wayland/RSurface.h>

#include <LCompositor.h>
#include <LClient.h>
#include <LWayland.h>
#include <LSeat.h>
#include <LToplevelRole.h>
#include <LPopupRole.h>
#include <LSubsurfaceRole.h>
#include <LPointer.h>
#include <LOutput.h>

#include <time.h>
#include <stdlib.h>

#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <unistd.h>
#include <LLog.h>
#include <LTime.h>

using namespace Louvre::Protocols::Wayland;

PFNEGLQUERYWAYLANDBUFFERWL eglQueryWaylandBufferWL = NULL;

LCursorRole *LSurface::cursor() const
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

LSurface::LSurface(Louvre::LSurface::Params *params, GLuint textureUnit)
{
    m_imp = new LSurfacePrivate();
    eglQueryWaylandBufferWL = (PFNEGLQUERYWAYLANDBUFFERWL) eglGetProcAddress ("eglQueryWaylandBufferWL");
    imp()->texture = new LTexture(params->surfaceResource->compositor(), textureUnit);
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
    imp()->minimized = state;
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

bool LSurface::minimized() const
{
    return imp()->minimized;
}

LSeat *LSurface::seat() const
{
    return compositor()->seat();
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
    {
        LPoint &sp = (LPoint&)role()->rolePosC();

        if (compositor()->globalScale() != 1)
        {
            imp()->posC.setX(imp()->posC.x() + sp.x() % compositor()->globalScale());
            imp()->posC.setY(imp()->posC.y() + sp.y() % compositor()->globalScale());
        }

        return role()->rolePosC();
    }


    LPoint &sp = imp()->posC;

    if (compositor()->globalScale() != 1)
    {
        imp()->posC.setX(imp()->posC.x() + sp.x() % compositor()->globalScale());
        imp()->posC.setY(imp()->posC.y() + sp.y() % compositor()->globalScale());
    }

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
        if (g->output() == output)
            return wl_surface_send_enter(surfaceResource()->resource(), g->resource());
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
                if (g->output() == output)
                    return wl_surface_send_leave(surfaceResource()->resource(), g->resource());
            return;
        }
    }
}

const list<LOutput *> &LSurface::outputs() const
{
    return imp()->outputs;
}


void LSurface::requestNextFrame()
{
    imp()->currentDamagesB.clear();
    imp()->currentDamagesC.clear();
    imp()->damaged = false;

    if (imp()->frameCallback)
    {
        wl_callback_send_done(imp()->frameCallback, LTime::ms());
        wl_resource_destroy(imp()->frameCallback);
        imp()->frameCallback = nullptr;
    }
}

bool LSurface::mapped() const
{
    return imp()->mapped && roleId() != Undefined;
}

Protocols::Wayland::RSurface *LSurface::surfaceResource() const
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

LCompositor *LSurface::compositor() const
{
    if (client() != nullptr)
        return client()->compositor();
    else
        return nullptr;
}

Louvre::LSurface *LSurface::parent() const
{
    return imp()->parent;
}

LSurface *findTopmostParent(LSurface *surface)
{
    if (surface->parent() == nullptr)
        return surface;

    return findTopmostParent(surface->parent());
}
Louvre::LSurface *LSurface::topmostParent() const
{
    if (parent() == nullptr)
        return nullptr;

    return findTopmostParent(parent());
}

const list<Louvre::LSurface *> &LSurface::children() const
{
    return imp()->children;
}

// Private
void LSurface::LSurfacePrivate::setParent(LSurface *parent)
{
    if (parent == this->parent)
        return;

    LSurface *surface = surfaceResource->surface();

    if (parent == nullptr)
    {
        this->parent->imp()->removeChild(surface);
        return;
    }

    this->parent = parent;


    if (parent->children().empty())
    {
        surface->compositor()->imp()->insertSurfaceAfter(parent, surface);
    }
    else
    {
        surface->compositor()->imp()->insertSurfaceAfter(parent->children().back(), surface);
    }

    parent->imp()->children.push_back(surface);
    surface->parentChanged();

    if (surface->role())
        surface->role()->handleParentChange();
}

void LSurface::LSurfacePrivate::removeChild(LSurface *child)
{
    children.remove(child);
    child->imp()->parent = nullptr;
    child->parentChanged();
}

void LSurface::LSurfacePrivate::setMapped(bool state)
{
    LSurface *surface = surfaceResource->surface();

    bool before = surface->mapped();

    mapped = state;

    if (before != surface->mapped())
    {
        surface->mappingChanged();

        list<LSurface*> childrenTmp = children;

        for (LSurface *c : childrenTmp)
        {
            if (c->role())
                c->role()->handleParentMappingChange();
        }
    }
}

void LSurface::LSurfacePrivate::setPendingRole(LBaseSurfaceRole *role)
{
    pending.role = role;
}

void LSurface::LSurfacePrivate::applyPendingRole()
{
    LSurface *surface = surfaceResource->surface();
    current.role = pending.role;
    pending.role = nullptr;
    surface->roleChanged();
}

void LSurface::LSurfacePrivate::applyPendingChildren()
{
    LSurface *surface = surfaceResource->surface();

    while(!pendingChildren.empty())
    {
        LSurface *child = pendingChildren.front();
        pendingChildren.pop_front();

        if (surface->children().empty())
            surface->compositor()->imp()->insertSurfaceAfter(surface,child);
        else
            surface->compositor()->imp()->insertSurfaceAfter(surface->children().back(),child);

        children.push_back(child);
        child->imp()->pendingParent = nullptr;
        child->imp()->parent = surface;
        child->parentChanged();
        if (child->role())
            child->role()->handleParentChange();
    }
}

bool LSurface::LSurfacePrivate::bufferToTexture()
{
    GLint texture_format;
    Int32 width, height;
    bool bufferScaleChanged = false;
    LSurface *surface = surfaceResource->surface();
    LSize prevSize = texture->sizeB();

    /***********************************
     *********** BUFFER SCALE ***********
     ***********************************/

    if (surface->imp()->current.bufferScale != surface->imp()->pending.bufferScale)
    {
        surface->imp()->current.bufferScale = surface->imp()->pending.bufferScale;
        bufferScaleChanged = true;
        surface->bufferScaleChanged();
    }

    // SHM
    if (wl_shm_buffer_get(current.buffer))
    {     
        if (texture && texture != textureBackup && texture->imp()->pendingDelete)
            delete texture;

        texture = textureBackup;
        wl_shm_buffer *shm_buffer = wl_shm_buffer_get(current.buffer);
        wl_shm_buffer_begin_access(shm_buffer);
        width = wl_shm_buffer_get_width(shm_buffer);
        height = wl_shm_buffer_get_height(shm_buffer);
        void *data = wl_shm_buffer_get_data(shm_buffer);
        UInt32 format =  LTexture::waylandFormatToDRM(wl_shm_buffer_get_format(shm_buffer));
        Int32 stride = wl_shm_buffer_get_stride(shm_buffer);
        LSize newSize = LSize(width, height);

        if (texture->sizeB() != newSize)
            bufferSizeChanged = true;

        // Reemplaza toda la textura
        if (!texture->initialized() || bufferSizeChanged || bufferScaleChanged)
        {
            currentDamagesB.clear();
            currentDamagesB.addRect(LRect(0,newSize));
            currentDamagesC = currentDamagesB;
            currentDamagesC.multiply(float(surface->compositor()->globalScale())/float(surface->bufferScale()));
            texture->setDataB(newSize, stride, format, data);
        }

        // Aplica daños
        else if (!pendingDamagesB.empty() || !pendingDamagesS.empty())
        {
            LRegion onlyPending = pendingDamagesS;
            onlyPending.multiply(current.bufferScale);
            onlyPending.addRegion(pendingDamagesB);
            onlyPending.clip(LRect(0,texture->sizeB()));
            UInt32 pixelSize = LTexture::formatBytesPerPixel(format);
            UChar8 *buff = (UChar8 *)data;

            for (const LRect &r : onlyPending.rects())
                texture->updateRect(r, stride, &buff[r.x()*pixelSize +r.y()*stride]);

            currentDamagesB.addRegion(onlyPending);
            currentDamagesC = currentDamagesB;
            currentDamagesC.multiply(float(surface->compositor()->globalScale())/float(surface->bufferScale()));
        }
        else
        {
            wl_shm_buffer_end_access(shm_buffer);
            return true;
        }

        wl_shm_buffer_end_access(shm_buffer);
    }
    // WL_DRM
    else if (eglQueryWaylandBufferWL(LWayland::eglDisplay(), current.buffer, EGL_TEXTURE_FORMAT, &texture_format))
    {
        if (texture && texture != textureBackup && texture->imp()->pendingDelete)
            delete texture;

        texture = textureBackup;

        eglQueryWaylandBufferWL(LWayland::eglDisplay(), current.buffer, EGL_WIDTH, &width);
        eglQueryWaylandBufferWL(LWayland::eglDisplay(), current.buffer, EGL_HEIGHT, &height);

        LSize newSize = LSize(width, height);

        if (newSize != prevSize || bufferScaleChanged)
        {
            bufferSizeChanged = true;
            currentDamagesB.clear();
            currentDamagesB.addRect(LRect(0,newSize));
            currentDamagesC = currentDamagesB;
            currentDamagesC.multiply(float(surface->compositor()->globalScale())/float(surface->bufferScale()));
        }
        else if (!pendingDamagesB.empty() || !pendingDamagesS.empty())
        {
            pendingDamagesS.multiply(surface->bufferScale());
            currentDamagesB.addRegion(pendingDamagesS);
            currentDamagesB.addRegion(pendingDamagesB);
            currentDamagesB.clip(LRect(0,newSize));
            currentDamagesC = currentDamagesB;
            currentDamagesC.multiply(float(surface->compositor()->globalScale())/float(surface->bufferScale()));
        }

        texture->setData(current.buffer);
    }
    else if (isDMABuffer(current.buffer))
    {
        LDMABuffer *dmaBuffer = (LDMABuffer*)wl_resource_get_user_data(current.buffer);
        width = dmaBuffer->planes()->width;
        height = dmaBuffer->planes()->height;

        if (!dmaBuffer->texture())
        {
            dmaBuffer->imp()->texture = new LTexture(surface->compositor(), texture->unit());
            dmaBuffer->texture()->setDataB(dmaBuffer->planes());
        }

        LSize newSize = LSize(width, height);

        if (newSize != prevSize || bufferScaleChanged || !texture->initialized())
        {
            bufferSizeChanged = true;
            currentDamagesB.clear();
            currentDamagesB.addRect(LRect(0,newSize));
            currentDamagesC = currentDamagesB;
            currentDamagesC.multiply(float(surface->compositor()->globalScale())/float(surface->bufferScale()));
        }
        else if (!pendingDamagesB.empty() || !pendingDamagesS.empty())
        {
            pendingDamagesS.multiply(surface->bufferScale());
            currentDamagesB.addRegion(pendingDamagesS);
            currentDamagesB.addRegion(pendingDamagesB);
            currentDamagesB.clip(LRect(0,newSize));
            currentDamagesC = currentDamagesB;
            currentDamagesC.multiply(float(surface->compositor()->globalScale())/float(surface->bufferScale()));
        }

        if (texture && texture != textureBackup && texture->imp()->pendingDelete)
            delete texture;

        texture = dmaBuffer->texture();
    }
    else
    {
        printf("Unknown buffer type.\n");
        texture->setData(current.buffer);
        exit(1);

        //wl_client_destroy(client->client());
        return false;
    }

    currentSizeB = texture->sizeB();
    currentSizeS = texture->sizeB()/current.bufferScale;
    currentSizeC = (texture->sizeB()*surface->compositor()->globalScale())/current.bufferScale;

    if (bufferSizeChanged)
        surface->bufferSizeChanged();

    pendingDamagesB.clear();
    pendingDamagesS.clear();

    wl_buffer_send_release(current.buffer);
    bufferReleased = true;
    damaged = true;
    surface->damaged();
    return true;
}

void LSurface::LSurfacePrivate::globalScaleChanged(Int32 oldScale, Int32 newScale)
{
    L_UNUSED(oldScale);

    LSurface *surface = surfaceResource->surface();

    // Size
    currentSizeC = (currentSizeB*newScale)/current.bufferScale;

    // Input region
    currentInputRegionC = currentInputRegionS;
    currentInputRegionC.multiply(newScale);

    // Opaque region
    surface->imp()->currentOpaqueRegionC = surface->imp()->currentOpaqueRegionS;
    surface->imp()->currentOpaqueRegionC.multiply(newScale);

    // Translucent region
    surface->imp()->currentTranslucentRegionC = surface->imp()->currentTranslucentRegionS;
    surface->imp()->currentTranslucentRegionC.multiply(newScale);

    // Damages
    currentDamagesC = currentDamagesB;
    currentDamagesC.multiply(float(surface->compositor()->globalScale())/float(surface->bufferScale()));

    // Role
    if (surface->role())
        surface->role()->globalScaleChanged(oldScale, newScale);
}

void LSurface::LSurfacePrivate::sendPresentationFeedback(LOutput *output, timespec &ns)
{
    if (wpPresentationFeedbackResources.empty())
        return;

    // Check if the surface is visible in the given output
    for (Wayland::GOutput *gOutput : surfaceResource->client()->outputGlobals())
    {
        if (gOutput->output() == output)
        {
            UInt32 tv_sec_hi = ns.tv_sec >> 32;
            UInt32 tv_sec_lo = ns.tv_sec & 0xFFFFFFFF;
            UInt32 seq_hi = output->imp()->presentationSeq >> 32;
            UInt32 seq_lo = output->imp()->presentationSeq & 0xFFFFFFFF;
            UInt32 refresh = 1000000000000/output->currentMode()->refreshRate();

            while (!wpPresentationFeedbackResources.empty())
            {
                WpPresentationTime::RWpPresentationFeedback *rFeed = wpPresentationFeedbackResources.back();
                rFeed->sync_output(gOutput);
                rFeed->presented(tv_sec_hi,
                                 tv_sec_lo,
                                 ns.tv_nsec,
                                 refresh,
                                 seq_hi,
                                 seq_lo,
                                 WP_PRESENTATION_FEEDBACK_KIND_VSYNC);
                rFeed->imp()->lSurface = nullptr;
                wpPresentationFeedbackResources.pop_back();
                wl_resource_destroy(rFeed->resource());
            }
            return;
        }
    }

    if (outputs.empty())
    {
        while (!wpPresentationFeedbackResources.empty())
        {
            WpPresentationTime::RWpPresentationFeedback *rFeed = wpPresentationFeedbackResources.back();
            rFeed->discarded();
            rFeed->imp()->lSurface = nullptr;
            wpPresentationFeedbackResources.pop_back();
            wl_resource_destroy(rFeed->resource());
        }
    }
}

