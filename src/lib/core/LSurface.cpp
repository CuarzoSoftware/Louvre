#include "LOutputMode.h"
#include "drm_fourcc.h"

#include <private/LSurfacePrivate.h>
#include <private/LCompositorPrivate.h>
#include <private/LOutputPrivate.h>

#include <protocols//PresentationTime/presentation-time.h>
#include <protocols/XdgShell/xdg-shell.h>
#include <protocols/DMABuffer/DMA.h>

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

using namespace Louvre;

PFNEGLQUERYWAYLANDBUFFERWL eglQueryWaylandBufferWL = NULL;

LCursorRole *LSurface::cursor() const
{
    if(roleId() == LSurface::Role::Cursor)
        return (LCursorRole*)imp()->current.role;
    else
        return nullptr;
}

LToplevelRole *LSurface::toplevel() const
{
    if(roleId() == LSurface::Role::Toplevel)
        return (LToplevelRole*)imp()->current.role;
    else
        return nullptr;
}

LPopupRole *LSurface::popup() const
{
    if(roleId() == LSurface::Role::Popup)
        return (LPopupRole*)imp()->current.role;
    else
        return nullptr;
}

LSubsurfaceRole *LSurface::subsurface() const
{
    if(roleId() == LSurface::Role::Subsurface)
        return (LSubsurfaceRole*)imp()->current.role;
    else
        return nullptr;
}

LDNDIconRole *LSurface::dndIcon() const
{
    if(roleId() == LSurface::Role::DNDIcon)
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
    imp()->surface = this;
    imp()->texture = new LTexture(textureUnit);
    imp()->resource = params->surface;
    imp()->client = params->client;
}

LSurface::~LSurface()
{
    delete imp()->texture;
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
    for(LOutput *o : outputs())
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
    if(role())
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
    if(role())
    {
        LPoint &sp = (LPoint&)role()->rolePosC();

        if(compositor()->globalScale() != 1)
        {
            imp()->posC.setX(imp()->posC.x() + sp.x() % compositor()->globalScale());
            imp()->posC.setY(imp()->posC.y() + sp.y() % compositor()->globalScale());
        }

        return role()->rolePosC();
    }


    LPoint &sp = imp()->posC;

    if(compositor()->globalScale() != 1)
    {
        imp()->posC.setX(imp()->posC.x() + sp.x() % compositor()->globalScale());
        imp()->posC.setY(imp()->posC.y() + sp.y() % compositor()->globalScale());
    }

    return imp()->posC;
}

void LSurface::sendOutputEnterEvent(LOutput *output)
{
    // Verfica si ya existe
    list<LOutput*>::const_iterator o;
    for(o = imp()->outputs.begin(); o != imp()->outputs.end(); o++)
        if(*o == output)
            return;

    imp()->outputs.push_back(output);

    // Verifica que haya creado un recurso para la salida
    for(wl_resource *r : client()->outputs())
    {
        if(wl_resource_get_user_data(r) == output)
        {
            wl_surface_send_enter(resource(),r);
            return;
        }
    }
}

void LSurface::sendOutputLeaveEvent(LOutput *output)
{
    // Verfica si ya existe
    list<LOutput*>::iterator o;
    for(o = imp()->outputs.begin(); o != imp()->outputs.end(); o++)
    {
        if(*o == output)
        {
            imp()->outputs.erase(o);

            // Verifica que haya creado un recurso para la salida
            for(wl_resource *r : client()->outputs())
            {
                if(wl_resource_get_user_data(r) == output)
                {
                    wl_surface_send_leave(resource(),r);
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


void LSurface::requestNextFrame(LOutput *output)
{
    imp()->currentDamagesB.clear();
    imp()->currentDamagesC.clear();
    imp()->damaged = false;

    if(imp()->frameCallback)
    {
        wl_callback_send_done(imp()->frameCallback, LTime::ms());
        wl_resource_destroy(imp()->frameCallback);
        imp()->frameCallback = nullptr;
    }

    if(!imp()->presentationOutputResources.empty())
        return;

    // If not specified, use the output where the largest area of the surface is visible
    if(!output)
    {
        Int32 maxArea = 0;
        Int32 area;

        for(LOutput *out : compositor()->outputs())
        {
            LRegion reg;
            reg.addRect(out->rectC());
            reg.clip(LRect(rolePosC(), sizeC()));
            if(reg.empty())
                continue;
            area = reg.rects().front().area();

            if(area > maxArea)
            {
                output = out;
                maxArea = area;
            }
        }
    }

    if(!output)
        return;

    for(wl_resource *out : client()->outputs())
        if(wl_resource_get_user_data(out) == output)
            imp()->presentationOutputResources.push_back(out);

    imp()->presentationOutput = output;
}

bool LSurface::mapped() const
{
    return imp()->mapped && roleId() != Undefined;
}

wl_buffer *LSurface::buffer() const
{
    return (wl_buffer*)imp()->current.buffer;
}

wl_resource *LSurface::resource() const
{
    return imp()->resource;
}

wl_resource *LSurface::xdgSurfaceResource() const
{
    return imp()->xdgSurfaceResource;
}

LClient *LSurface::client() const
{
    return imp()->client;
}

LCompositor *LSurface::compositor() const
{
    if(imp()->client != nullptr)
        return imp()->client->compositor();
    else
        return nullptr;
}

Louvre::LSurface *LSurface::parent() const
{
    return imp()->parent;
}

LSurface *findTopmostParent(LSurface *surface)
{
    if(surface->parent() == nullptr)
        return surface;

    return findTopmostParent(surface->parent());
}
Louvre::LSurface *LSurface::topmostParent() const
{
    if(parent() == nullptr)
        return nullptr;

    return findTopmostParent(parent());
}

const list<Louvre::LSurface *> &LSurface::children() const
{
    return imp()->children;
}

LSurface::LSurfacePrivate *LSurface::imp() const
{
    return m_imp;
}

// Private
void LSurface::LSurfacePrivate::setParent(LSurface *parent)
{
    if(parent == this->parent)
        return;


    if(parent == nullptr)
    {
        this->parent->imp()->removeChild(surface);
        return;
    }

    this->parent = parent;


    if(parent->children().empty())
    {
        surface->compositor()->imp()->insertSurfaceAfter(parent, surface);
    }
    else
    {
        surface->compositor()->imp()->insertSurfaceAfter(parent->children().back(), surface);
    }

    parent->imp()->children.push_back(surface);
    surface->parentChanged();

    if(surface->role())
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
    bool before = surface->mapped();

    mapped = state;

    if(before != surface->mapped())
    {
        surface->mappingChanged();

        list<LSurface*> childrenTmp = children;

        for(LSurface *c : childrenTmp)
        {
            if(c->role())
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
    current.role = pending.role;
    pending.role = nullptr;
    surface->roleChanged();
}

void LSurface::LSurfacePrivate::applyPendingChildren()
{
    while(!pendingChildren.empty())
    {
        LSurface *child = pendingChildren.front();
        pendingChildren.pop_front();

        if(surface->children().empty())
            surface->compositor()->imp()->insertSurfaceAfter(surface,child);
        else
            surface->compositor()->imp()->insertSurfaceAfter(surface->children().back(),child);

        children.push_back(child);
        child->imp()->pendingParent = nullptr;
        child->imp()->parent = surface;
        child->parentChanged();
        if(child->role())
            child->role()->handleParentChange();
    }
}

bool LSurface::LSurfacePrivate::bufferToTexture()
{

    Int32 width, height;
    EGLint texture_format;
    bool bufferScaleChanged = false;

    /***********************************
     *********** BUFFER SCALE ***********
     ***********************************/

    if(surface->imp()->current.bufferScale != surface->imp()->pending.bufferScale)
    {
        surface->imp()->current.bufferScale = surface->imp()->pending.bufferScale;
        bufferScaleChanged = true;
        surface->bufferScaleChanged();
    }

    // EGL
    if(eglQueryWaylandBufferWL(LWayland::eglDisplay(), current.buffer, EGL_TEXTURE_FORMAT, &texture_format))
    {
        eglQueryWaylandBufferWL(LWayland::eglDisplay(), current.buffer, EGL_WIDTH, &width);
        eglQueryWaylandBufferWL(LWayland::eglDisplay(), current.buffer, EGL_HEIGHT, &height);

        LSize newSize = LSize(width, height);

        if(texture->sizeB() != newSize || bufferScaleChanged)
        {
            bufferSizeChanged = true;
            currentDamagesB.clear();
            currentDamagesB.addRect(LRect(0,newSize));
            currentDamagesC = currentDamagesB;
            currentDamagesC.multiply(float(surface->compositor()->globalScale())/float(surface->bufferScale()));
        }
        else if(!pendingDamagesB.empty() || !pendingDamagesS.empty())
        {
            pendingDamagesS.multiply(surface->bufferScale());
            currentDamagesB.addRegion(pendingDamagesS);
            currentDamagesB.addRegion(pendingDamagesB);
            currentDamagesB.clip(LRect(0,newSize));
            currentDamagesC = currentDamagesB;
            currentDamagesC.multiply(float(surface->compositor()->globalScale())/float(surface->bufferScale()));
        }


        EGLAttrib attribs = EGL_NONE;
        EGLImage image = eglCreateImage(LWayland::eglDisplay(), EGL_NO_CONTEXT, EGL_WAYLAND_BUFFER_WL, current.buffer, &attribs);
        texture->setDataB(width, height, &image, texture_format, GL_UNSIGNED_BYTE, LTexture::BufferSourceType::EGL);
        eglDestroyImage(LWayland::eglDisplay(), image);

    }
    // SHM
    else if(wl_shm_buffer_get(current.buffer))
    {

        wl_shm_buffer *shm_buffer = wl_shm_buffer_get(current.buffer);
        wl_shm_buffer_begin_access(shm_buffer);
        width = wl_shm_buffer_get_width(shm_buffer);
        height = wl_shm_buffer_get_height(shm_buffer);
        void *data = wl_shm_buffer_get_data(shm_buffer);
        UInt32 format =  wl_shm_buffer_get_format(shm_buffer);

        GLenum bufferFormat, bufferType;

        if(!LWayland::wlFormat2Gl(format, &bufferFormat, &bufferType))
        {
            printf("Unsupported buffer format.\n");
            wl_shm_buffer_end_access(shm_buffer);
            wl_client_destroy(client->client());
            return false;
        }

        LSize newSize = LSize(width, height);

        if(texture->sizeB() != newSize)
            bufferSizeChanged = true;


        // Reemplaza toda la textura
        if(!texture->initialized() || bufferSizeChanged || bufferFormat != texture->format() || bufferType != texture->type() || bufferScaleChanged)
        {
            currentDamagesB.clear();
            currentDamagesB.addRect(LRect(0,newSize));
            currentDamagesC = currentDamagesB;
            currentDamagesC.multiply(float(surface->compositor()->globalScale())/float(surface->bufferScale()));
            texture->setDataB(width, height, data, bufferFormat, bufferType);
        }

        // Aplica daños
        else if(!pendingDamagesB.empty() || !pendingDamagesS.empty())
        {
            LRegion onlyPending = pendingDamagesS;
            onlyPending.multiply(current.bufferScale);
            onlyPending.addRegion(pendingDamagesB);
            onlyPending.clip(LRect(0,texture->sizeB()));

            glBindTexture(GL_TEXTURE_2D, texture->id());

            for(const LRect &r : onlyPending.rects())
            {

                glPixelStorei(GL_UNPACK_ROW_LENGTH,texture->sizeB().w());
                glPixelStorei(GL_UNPACK_SKIP_PIXELS,(GLint)r.x());
                glPixelStorei(GL_UNPACK_SKIP_ROWS,(GLint)r.y());


                glTexSubImage2D(GL_TEXTURE_2D,
                                0,
                                (GLint)r.x(),
                                (GLint)r.y(),
                                (GLsizei)r.w(),
                                (GLsizei)r.h(),
                                texture->format(),
                                texture->type(),
                                data);
            }

            glPixelStorei(GL_UNPACK_ROW_LENGTH,0);
            glPixelStorei(GL_UNPACK_SKIP_PIXELS,0);
            glPixelStorei(GL_UNPACK_SKIP_ROWS,0);

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
    else if(wl_buffer_is_dmabuf(current.buffer))
    {
        LDMABuffer *dma = (LDMABuffer*)wl_resource_get_user_data(current.buffer);
        width = dma->width;
        height = dma->height;

        LLog::log("W:%d H:%d", width, height);

        LSize newSize = LSize(width, height);

        if(texture->sizeB() != newSize || bufferScaleChanged)
        {
            bufferSizeChanged = true;
            currentDamagesB.clear();
            currentDamagesB.addRect(LRect(0,newSize));
            currentDamagesC = currentDamagesB;
            currentDamagesC.multiply(float(surface->compositor()->globalScale())/float(surface->bufferScale()));
        }
        else if(!pendingDamagesB.empty() || !pendingDamagesS.empty())
        {
            pendingDamagesS.multiply(surface->bufferScale());
            currentDamagesB.addRegion(pendingDamagesS);
            currentDamagesB.addRegion(pendingDamagesB);
            currentDamagesB.clip(LRect(0,newSize));
            currentDamagesC = currentDamagesB;
            currentDamagesC.multiply(float(surface->compositor()->globalScale())/float(surface->bufferScale()));
        }

        texture->setDataB(width, height, &dma->eglImage, GL_BGRA, GL_UNSIGNED_BYTE, LTexture::BufferSourceType::EGL);

        //eglDestroyImage(LWayland::eglDisplay(), image);


    }
    else
    {
        printf("Unknown buffer type.\n");
        //wl_client_destroy(client->client());
        return false;
    }

    currentSizeB = texture->sizeB();
    currentSizeS = texture->sizeB()/current.bufferScale;
    currentSizeC = (texture->sizeB()*surface->compositor()->globalScale())/current.bufferScale;

    if(bufferSizeChanged)
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
    if(surface->role())
        surface->role()->globalScaleChanged(oldScale, newScale);
}

void LSurface::LSurfacePrivate::sendPresentationFeedback(LOutput *output, timespec &ns)
{
    if(output != presentationOutput)
        return;

    if(presentationFeedback.empty())
        return;

    if(presentationOutput)
    {
        UInt32 tv_sec_hi = ns.tv_sec >> 32;
        UInt32 tv_sec_lo = ns.tv_sec & 0xFFFFFFFF;
        UInt32 seq_hi = presentationOutput->imp()->presentationSeq >> 32;
        UInt32 seq_lo = presentationOutput->imp()->presentationSeq & 0xFFFFFFFF;
        UInt32 refresh = 1000000000/(presentationOutput->currentMode()->refreshRate()/1000);

        for(wl_resource *pres : presentationFeedback)
        {
            for(wl_resource *out : presentationOutputResources)
                wp_presentation_feedback_send_sync_output(pres, out);

            wp_presentation_feedback_send_presented(pres,
                                                    tv_sec_hi,
                                                    tv_sec_lo,
                                                    ns.tv_nsec,
                                                    refresh,
                                                    seq_hi,
                                                    seq_lo,
                                                    WP_PRESENTATION_FEEDBACK_KIND_VSYNC
                                                    );
            wl_resource_destroy(pres);

        }
    }
    else
    {
        for(wl_resource *pres : presentationFeedback)
        {
            wp_presentation_feedback_send_discarded(pres);
            wl_resource_destroy(pres);
        }
    }

    presentationFeedback.clear();
    presentationOutputResources.clear();
    presentationOutput = nullptr;

}

