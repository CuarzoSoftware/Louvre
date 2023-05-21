#include "LLog.h"
#include <fcntl.h>
#include <gbm.h>
#include <private/LCompositorPrivate.h>
#include <private/LClientPrivate.h>
#include <private/LSurfacePrivate.h>
#include <private/LToplevelRolePrivate.h>
#include <private/LDNDManagerPrivate.h>
#include <private/LSeatPrivate.h>

#include <protocols/Wayland/private/SeatGlobalPrivate.h>

#include <protocols/Wayland/RegionResource.h>
#include <protocols/Wayland/CompositorGlobal.h>
#include <protocols/Wayland/Subcompositor.h>
#include <protocols/Wayland/SurfaceResource.h>
#include <protocols/Wayland/DataDeviceManagerGlobal.h>
#include <protocols/Wayland/Output.h>

#include <protocols/XdgShell/XdgWmBase.h>
#include <protocols/XdgShell/xdg-shell.h>

#include <protocols/XdgDecoration/XdgDecorationManager.h>
#include <protocols/XdgDecoration/xdg-decoration-unstable-v1.h>

#include <protocols/DMABuffer/LinuxDMABuffer.h>
#include <protocols/DMABuffer/linux-dmabuf-unstable-v1.h>

#include <protocols/PresentationTime/Presentation.h>
#include <protocols/PresentationTime/presentation-time.h>

#include <LTexture.h>
#include <LWayland.h>
#include <LPainter.h>
#include <LOutput.h>

#include <pthread.h>
#include <sys/poll.h>
#include <unistd.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <unordered_map>
#include <sys/eventfd.h>


#include <protocols/DMABuffer/LGbm.h>


using namespace std;
using namespace Louvre;

PFNEGLBINDWAYLANDDISPLAYWL eglBindWaylandDisplayWL = NULL;

LCompositor *compositor;

static struct wl_display *display;
struct wl_event_loop *event_loop;
int wayland_fd;
wl_event_source *renderTimer;
wl_listener clientConnectedListener;
wl_event_source *clientDisconnectedEventSource;
wl_signal sign;
wl_listener signListen;
EGLContext eglCtx;
EGLDisplay eglDpy;
EGLContext sharedContext;
EGLDisplay sharedDisplay;
Int32 contextInitialized = 0;
const LOutput *m_mainOutput;

struct GLBufferType
{
    GLenum format;
    GLenum type;
};

unordered_map<UInt32, struct GLBufferType> shmFormats;

void LWayland::initGLContext()
{

    if(contextInitialized == 1)
        contextInitialized = 2;
    else
    {
        return;
    }

    static const EGLint attribs[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RED_SIZE, 0,
        EGL_GREEN_SIZE, 0,
        EGL_BLUE_SIZE, 0,
        EGL_ALPHA_SIZE, 0,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_NONE
    };

    static const EGLint context_attribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };

    eglDpy = sharedDisplay;

    EGLConfig config;
    EGLint num_configs_returned;

    eglChooseConfig(eglDpy, attribs, &config, 1, &num_configs_returned);

    // EGL context and surface
    if(!eglBindAPI(EGL_OPENGL_ES_API))
    {
        //printf("Failed to bind api EGL_OPENGL_ES_API\n");
        exit(-1);
    }

    // 3. Create a context and make it current
    eglCtx = eglCreateContext(eglDpy, config, sharedContext,context_attribs);

    if(eglCtx == NULL)
    {
        //printf("CONTEXT ERROR %d\n",eglGetError());
        exit(0);
    }


    eglMakeCurrent(eglDpy, EGL_NO_SURFACE, EGL_NO_SURFACE, eglCtx);

    compositor->imp()->painter = new LPainter();
    return;
}

bool LWayland::isGlContextInitialized()
{
    return contextInitialized == 2;
}

void LWayland::setContext(const LOutput *output, EGLDisplay sharedDisp, EGLContext sharedCont)
{
    m_mainOutput = output;
    sharedDisplay = sharedDisp;
    sharedContext = sharedCont;
    contextInitialized = 1;
}

void LWayland::setMainOutput(LOutput *output)
{
    m_mainOutput = output;
}

const LOutput *LWayland::mainOutput()
{
    return m_mainOutput;
}

LCompositor *LWayland::bindedCompositor()
{
    return compositor;
}

wl_event_source *LWayland::addFdListener(int fd, void *userData, int (*callback)(int, unsigned int, void *), UInt32 flags)
{
    return wl_event_loop_add_fd(event_loop, fd, flags, callback, userData);
}

void LWayland::removeFdListener(wl_event_source *source)
{
    wl_event_source_remove(source);
}

void removeFdListener(wl_event_source *source)
{
    wl_event_source_remove(source);
}

void LWayland::setSeat(LSeat *seat)
{
    // Create seat global
    wl_global_create(display, &wl_seat_interface, LOUVRE_SEAT_VERSION, seat->compositor(), &SeatGlobal::SeatGlobalPrivate::bind);
}

UInt32 LWayland::nextSerial()
{
    return wl_display_next_serial(display);
}

bool LWayland::wlFormat2Gl(UInt32 wlFormat, GLenum *glFormat, GLenum *glType)
{
    if(shmFormats.find(wlFormat) == shmFormats.end())
        return false;

    auto data = shmFormats[wlFormat];

    *glFormat = data.format;
    *glType = data.type;
    return true;
}

void addSHMFormat(wl_display *dpy, UInt32 wlFormat, GLenum glFormat, GLenum glType)
{
    wl_display_add_shm_format(dpy, wlFormat);
    shmFormats[wlFormat] =
    {
        .format = glFormat,
        .type = glType
    };
}
int LWayland::initWayland(LCompositor *comp)
{
    eglBindWaylandDisplayWL = (PFNEGLBINDWAYLANDDISPLAYWL) eglGetProcAddress ("eglBindWaylandDisplayWL");

    // Stores compositor reference
    compositor = comp;

    // Create a new Wayland display
    display = wl_display_create();

    if (!display)
    {
        //printf("Unable to create Wayland display.\n");
        exit(EXIT_FAILURE);
    }
    const char *socket = getenv("LOUVRE_WAYLAND_DISPLAY");

    if(socket)
    {
        int socketFD = wl_display_add_socket(display, socket);
        wl_display_add_socket_fd(display, socketFD);
    }
    else
    {
        // Use wayland-0 socket by default
        socket = wl_display_add_socket_auto(display);

        if (!socket)
        {
            printf("Unable to add socket to Wayland display.\n");
            exit(EXIT_FAILURE);
        }
    }


    // GLOBALS

    wl_global_create(display, &wl_compositor_interface,
                     LOUVRE_COMPOSITOR_VERSION, comp, &Protocols::Wayland::CompositorGlobal::bind);

    wl_global_create(display, &wl_subcompositor_interface,
                     LOUVRE_SUBCOMPOSITOR_VERSION, comp, &Globals::Subcompositor::bind);

    wl_global_create(display, &wl_data_device_manager_interface,
                     LOUVRE_DATA_DEVICE_MANAGER_VERSION, comp, &Protocols::Wayland::DataDeviceManagerGlobal::bind);

    wl_global_create(display, &xdg_wm_base_interface,
                     LOUVRE_XDG_WM_BASE_VERSION, comp, &Extensions::XdgShell::WmBase::bind);

    wl_global_create(display, &zxdg_decoration_manager_v1_interface,
                     LOUVRE_XDG_DECORATION_MANAGER_VERSION, comp, &Extensions::XdgDecoration::Manager::bind);

    /*wl_global_create(display, &zwp_linux_dmabuf_v1_interface,
                     3, comp, &Extensions::LinuxDMABuffer::LinuxDMABuffer::bind);*/

    wl_global_create(display, &wp_presentation_interface,
                     1, comp, &Extensions::PresentationTime::Presentation::bind);

    wl_display_init_shm(display);

    shmFormats[WL_SHM_FORMAT_ARGB8888] =
    {
        .format = GL_BGRA,
        .type = GL_UNSIGNED_BYTE
    };

    shmFormats[WL_SHM_FORMAT_XRGB8888] =
    {
        .format = GL_BGRA,
        .type = GL_UNSIGNED_BYTE
    };

    /*
    addSHMFormat(display, WL_SHM_FORMAT_XBGR8888,       GL_RGBA,    GL_UNSIGNED_BYTE);
    addSHMFormat(display, WL_SHM_FORMAT_ABGR8888,       GL_RGBA,    GL_UNSIGNED_BYTE);


    addSHMFormat(display, WL_SHM_FORMAT_BGR888,         GL_RGB,     GL_UNSIGNED_BYTE);

    addSHMFormat(display, WL_SHM_FORMAT_RGBX4444,       GL_RGBA,     GL_UNSIGNED_SHORT_4_4_4_4);
    addSHMFormat(display, WL_SHM_FORMAT_RGBA4444,       GL_RGBA,    GL_UNSIGNED_SHORT_4_4_4_4);
    addSHMFormat(display, WL_SHM_FORMAT_RGBX5551,       GL_RGBA,    GL_UNSIGNED_SHORT_5_5_5_1);
    addSHMFormat(display, WL_SHM_FORMAT_RGBA5551,       GL_RGBA,    GL_UNSIGNED_SHORT_5_5_5_1);
    addSHMFormat(display, WL_SHM_FORMAT_RGB565,         GL_RGB,     GL_UNSIGNED_SHORT_5_6_5);

    addSHMFormat(display, WL_SHM_FORMAT_XBGR2101010,    GL_RGBA,    GL_UNSIGNED_INT_2_10_10_10_REV_EXT);
    addSHMFormat(display, WL_SHM_FORMAT_ABGR2101010,    GL_RGBA,    GL_UNSIGNED_INT_2_10_10_10_REV_EXT);
    addSHMFormat(display, WL_SHM_FORMAT_XBGR16161616F,  GL_RGBA,    GL_HALF_FLOAT_OES);
    addSHMFormat(display, WL_SHM_FORMAT_ABGR16161616F,  GL_RGBA,    GL_HALF_FLOAT_OES);
    */

    /*

    WL_SHM_FORMAT_BGRX4444
    WL_SHM_FORMAT_ARGB4444
    WL_SHM_FORMAT_ABGR4444
    WL_SHM_FORMAT_RGBA4444
    WL_SHM_FORMAT_BGRA4444
    WL_SHM_FORMAT_XRGB1555
    WL_SHM_FORMAT_XBGR1555
    WL_SHM_FORMAT_RGBX5551
    WL_SHM_FORMAT_BGRX5551
    WL_SHM_FORMAT_ARGB1555
    WL_SHM_FORMAT_ABGR1555
    WL_SHM_FORMAT_RGBA5551
    WL_SHM_FORMAT_BGRA5551
    WL_SHM_FORMAT_RGB565
    WL_SHM_FORMAT_BGR565
    WL_SHM_FORMAT_BGR888
    WL_SHM_FORMAT_XBGR8888
    WL_SHM_FORMAT_RGBX8888
    WL_SHM_FORMAT_BGRX8888
    WL_SHM_FORMAT_ABGR8888
    WL_SHM_FORMAT_RGBA8888
    WL_SHM_FORMAT_BGRA8888
    WL_SHM_FORMAT_XRGB2101010
    WL_SHM_FORMAT_XBGR2101010
    WL_SHM_FORMAT_RGBX1010102
    WL_SHM_FORMAT_BGRX1010102
    WL_SHM_FORMAT_ARGB2101010
    WL_SHM_FORMAT_ABGR2101010
    WL_SHM_FORMAT_RGBA1010102
    WL_SHM_FORMAT_BGRA1010102
    WL_SHM_FORMAT_R8
    WL_SHM_FORMAT_R16
    WL_SHM_FORMAT_RG88
    WL_SHM_FORMAT_GR88
    WL_SHM_FORMAT_RG1616
    WL_SHM_FORMAT_GR1616
    WL_SHM_FORMAT_XRGB16161616F
    WL_SHM_FORMAT_XBGR16161616F
    WL_SHM_FORMAT_ARGB16161616F
    WL_SHM_FORMAT_ABGR16161616F
    WL_SHM_FORMAT_XRGB8888_A8
    WL_SHM_FORMAT_XBGR8888_A8
    WL_SHM_FORMAT_RGBX8888_A8
    WL_SHM_FORMAT_BGRX8888_A8
    WL_SHM_FORMAT_RGB888_A8
    WL_SHM_FORMAT_BGR888_A8
    WL_SHM_FORMAT_RGB565_A8
    WL_SHM_FORMAT_BGR565_A8
    WL_SHM_FORMAT_XRGB16161616
    WL_SHM_FORMAT_XBGR16161616
    WL_SHM_FORMAT_ARGB16161616
    WL_SHM_FORMAT_ABGR16161616
    */

    event_loop = wl_display_get_event_loop(display);
    wayland_fd = wl_event_loop_get_fd(event_loop);

    comp->imp()->waylandFd = wl_event_loop_get_fd(event_loop);

    // Listen for client connections
    clientConnectedListener.notify = &clientConnectionEvent;
    wl_display_add_client_created_listener(display,&clientConnectedListener);

    return wayland_fd;

}

void LWayland::terminateDisplay()
{
    wl_display_terminate(display);
}

void LWayland::dispatchEvents()
{
    wl_event_loop_dispatch(event_loop, 0);
}

void LWayland::flushClients()
{
    wl_display_flush_clients(display);
}

int drm_fd;

void LWayland::bindEGLDisplay(EGLDisplay eglDisplay)
{
    sharedDisplay = eglDisplay;
    eglBindWaylandDisplayWL(eglDisplay, display);
}

pollfd fds[2];
bool forcedUpdate = false;

void LWayland::forceUpdate()
{
    if(!forcedUpdate)
    {
        eventfd_write(fds[1].fd, 1);
        forcedUpdate = true;
    }
}

void LWayland::runLoop()
{
    fds[0].events = POLLIN | POLLOUT | POLLHUP;
    fds[0].fd = wayland_fd;
    fds[0].revents = 0;
    fds[1].events = POLLIN;
    fds[1].fd = eventfd(0, EFD_CLOEXEC|EFD_NONBLOCK);
    fds[1].revents = 0;
    eventfd_t val;

    while(true)
    {
        poll(fds, 2, -1);

        compositor->imp()->renderMutex.lock();


        compositor->seat()->imp()->dispatchSeat();


        if(forcedUpdate)
        {
            eventfd_read(fds[1].fd, &val);
            if(compositor->imp()->inputBackend)
                compositor->imp()->inputBackend->forceUpdate(compositor->seat());
            forcedUpdate = false;
            if(!(fds[0].events & (POLLIN | POLLOUT | POLLHUP)))
            {
                compositor->imp()->renderMutex.unlock();
                continue;
            }
        }


        // Espera 5 iteraciones para eliminar un global wl_output (ver documentación de wl_global_destroy)
        for(list<LCompositor::LCompositorPrivate::RemovedOutputGlobal*>::iterator g = compositor->imp()->removedOutputGobals.begin();
            g != compositor->imp()->removedOutputGobals.end(); g++)
        {
            if((*g)->loopIterations >= 5)
            {
                wl_global_destroy((*g)->global);
                delete (*g);
                g = compositor->imp()->removedOutputGobals.erase(g);
            }
            else
            {
                (*g)->loopIterations++;
            }
        }


        // DND
        if(compositor->seat()->dndManager()->imp()->destDidNotRequestReceive >= 3)
            compositor->seat()->dndManager()->cancel();

        if(compositor->seat()->dndManager()->imp()->dropped && compositor->seat()->dndManager()->imp()->destDidNotRequestReceive < 3)
            compositor->seat()->dndManager()->imp()->destDidNotRequestReceive++;

        dispatchEvents();
        flushClients();

        compositor->imp()->renderMutex.unlock();

    }
}

int LWayland::drmFd()
{
    return drm_fd;
}

void LWayland::clientConnectionEvent(wl_listener *listener, void *data)
{
    (void)listener;

    wl_client *client = (wl_client*)data;

    LClient::Params *params = new LClient::Params;
    params->client = client;
    params->compositor = compositor;

    // Let the developer create his own client implementation
    LClient *newClient = compositor->createClientRequest(params);

    // Listen for client disconnection
    //wl_listener *clientDisconnectedListener = new wl_listener;
    //clientDisconnectedListener->notify = &clientDisconnectionEvent;
    //wl_client_add_destroy_listener(client,clientDisconnectedListener);
    wl_client_get_destroy_listener(client,&LWayland::clientDisconnectionEvent);

    // Append client to the compositor list
    compositor->imp()->clients.push_back(newClient);
}

wl_iterator_result resourceDestroyIterator(wl_resource *resource, void*)
{
    wl_resource_destroy(resource);
    return WL_ITERATOR_CONTINUE;
}

void LWayland::clientDisconnectionEvent(wl_listener *listener, void *data)
{
    L_UNUSED(listener);

    wl_client *client = (wl_client*)data;

    wl_client_for_each_resource(client, resourceDestroyIterator, NULL);

    LClient *disconnectedClient = nullptr;

    // Remove client from the compositor list
    for(LClient *wClient: compositor->clients())
    {
        if(wClient->client() == client)
        {
            disconnectedClient = wClient;
            break;
        }
    }

    if(disconnectedClient == nullptr)
        return;

    compositor->destroyClientRequest(disconnectedClient);
    compositor->imp()->clients.remove(disconnectedClient);

    delete disconnectedClient;
}


wl_display *LWayland::getDisplay()
{
    return display;
}

EGLContext LWayland::eglContext()
{
    return sharedContext;
}

EGLDisplay LWayland::eglDisplay()
{
    return sharedDisplay;
}
