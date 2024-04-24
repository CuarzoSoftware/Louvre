#include "LOutputMode.h"
#include "LTime.h"
#include <cstring>
#include <fcntl.h>
#include <private/LCompositorPrivate.h>
#include <private/LOutputPrivate.h>
#include <sys/mman.h>
#include <wayland-client.h>
#include <LLog.h>
#include <SRM/SRMFormat.h>
#include <xdg-shell-client.h>
#include <xdg-decoration-unstable-v1-client.h>
#include <wayland-egl.h>
#include <LCursor.h>

using namespace Louvre;

#define BKND_NAME "WAYLAND BACKEND"
#define CURSOR_SIZE 64 * 64 * 4

struct WaylandBackendShared
{
    std::mutex mutex;
    pollfd fd[3];
    LSize surfaceSize { 1024, 512 };
    LSize bufferSize { 1024, 512 };
    Int32 bufferScale { 1 };
    wl_buffer *cursorBuffer { nullptr };
    wl_surface *cursorSurface { nullptr };
    bool cursorChangedHotspot { false };
    bool cursorChangedBuffer { false };
    bool cursorVisible { false };
    LPoint cursorHotspot;
};

struct Texture
{
    GLuint id;
    GLenum target;
};

struct CPUTexture
{
    Texture texture;
    UInt32 pixelSize;
    const SRMGLFormat *glFmt;
};

struct DRMTexture
{
    Texture texture;
    EGLImage image;
};

struct WaylandOutput
{
    UInt32 name;
    Int32 bufferScale { 1 };
    Int32 refresh { 60000 };
};

static const EGLint eglContextAttribs[]
{
    EGL_CONTEXT_CLIENT_VERSION, 2,
    EGL_NONE
};

static EGLint eglConfigAttribs[]
{
    EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
    EGL_RED_SIZE, 8,
    EGL_GREEN_SIZE, 8,
    EGL_BLUE_SIZE, 8,
    EGL_ALPHA_SIZE, 0,
    EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
    EGL_NONE
};

class Louvre::LGraphicBackend
{
public:
    inline static WaylandBackendShared shared;
    inline static wl_display *display;
    inline static wl_registry *registry;
    inline static wl_compositor *compositor { nullptr };
    inline static xdg_wm_base *xdgWmBase { nullptr };
    inline static wl_surface *surface;
    inline static xdg_surface *xdgSurface;
    inline static xdg_toplevel *xdgToplevel;
    inline static wl_egl_window *eglWindow;
    inline static zxdg_decoration_manager_v1 *xdgDecorationManager { nullptr };
    inline static zxdg_toplevel_decoration_v1 *xdgDecoration { nullptr };
    inline static std::vector<wl_output*> waylandOutputs;
    inline static std::vector<wl_output*> surfaceOutputs;

    inline static wl_registry_listener registryListener;
    inline static wl_surface_listener surfaceListener;
    inline static wl_output_listener outputListener;
    inline static xdg_wm_base_listener xdgWmBaseListener;
    inline static xdg_surface_listener xdgSurfaceListener;
    inline static xdg_toplevel_listener xdgToplevelListener;
    inline static EGLDisplay eglDisplay;
    inline static EGLContext eglContext, windowEGLContext;
    inline static EGLConfig eglConfig;
    inline static EGLSurface eglSurface;
    inline static std::thread renderThread;
    inline static UInt32 refreshRate { 60000 };
    inline static Int32 refreshRateLimit { 0 };
    inline static LSize physicalSize { 0, 0 };
    inline static LSize pendingSurfaceSize { 1024, 512 };
    inline static Int32 pendingBufferScale { 1 };
    inline static std::vector<LOutput*> dummyOutputs;
    inline static std::vector<LOutputMode *> dummyOutputModes;
    inline static Int8 windowInitialized { 0 };
    inline static Int64 lastFrameUsec { 0 };
    inline static bool repaint { false };
    inline static bool vSync { true };

    // Cursor
    inline static wl_shm *shm { nullptr };
    inline static wl_shm_pool *shmPool { nullptr };
    inline static Int32 cursorMapFd { -1 };
    inline static UInt8 *cursorMap { nullptr };

    static UInt32 backendGetId()
    {
        return LGraphicBackendWayland;
    }

    static void *backendGetContextHandle()
    {
        return display;
    }

    static void unlockInputThread()
    {
        if (shared.fd[2].fd != -1)
            eventfd_write(shared.fd[2].fd, 1);
    }

    static bool initCursor()
    {
        if (!shm)
            return false;

        cursorMapFd = Louvre::createSHM(CURSOR_SIZE);

        if (cursorMapFd < 0)
            return false;

        cursorMap = (UInt8*)mmap(NULL, CURSOR_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, cursorMapFd, 0);

        if (cursorMap == MAP_FAILED)
        {
            cursorMap = nullptr;
            close(cursorMapFd);
            return false;
        }

        shmPool = wl_shm_create_pool(shm, cursorMapFd, CURSOR_SIZE);
        shared.cursorBuffer = wl_shm_pool_create_buffer(shmPool, 0, 64, 64, 64 * 4, WL_SHM_FORMAT_ARGB8888);
        shared.cursorSurface = wl_compositor_create_surface(compositor);
        wl_surface_attach(shared.cursorSurface, shared.cursorBuffer, 0, 0);
        wl_display_roundtrip(display);

        return true;
    }

    static bool initWayland()
    {
        display = wl_display_connect(NULL);

        if (!display)
        {
            LLog::fatal("[%s] Failed to create wl_display.", BKND_NAME);
            return false;
        }

        registryListener.global = registryHandleGlobal;
        registryListener.global_remove = registryHandleGlobalRemove;
        surfaceListener.enter = surfaceHandleEnter;
        surfaceListener.leave = surfaceHandleLeave;
        outputListener.geometry = [](auto, auto, auto, auto, auto, auto, auto, auto, auto, auto){};
        outputListener.mode = outputHandleMode;
        outputListener.scale = outputHandleScale;
        outputListener.done = outputHandleDone;
        xdgWmBaseListener.ping = xdgWmBaseHandlePing;
        xdgSurfaceListener.configure = xdgSurfaceHandleConfigure;
        xdgToplevelListener.close = xdgToplevelHandleClose;
        xdgToplevelListener.configure = xdgToplevelHandleConfigure;

        registry = wl_display_get_registry(display);
        wl_registry_add_listener(registry, &registryListener, nullptr);
        wl_display_dispatch(display);
        wl_display_roundtrip(display);

        if (!compositor)
        {
            LLog::fatal("[%s] Failed to get wl_compositor.", BKND_NAME);
            return false;
        }

        if (!xdgWmBase)
        {
            LLog::fatal("[%s] Failed to get xdg_wm_base.", BKND_NAME);
            return false;
        }

        initCursor();

        shared.fd[1].fd = wl_display_get_fd(display);
        shared.fd[1].events = WL_EVENT_READABLE | WL_EVENT_WRITABLE;
        shared.fd[1].revents = 0;

        return true;
    }

    static bool initEGL()
    {
        /* TODO: Check steps */
        EGLint major, minor, n;
        eglDisplay = eglGetDisplay(display);
        eglInitialize(eglDisplay, &major, &minor);
        eglBindAPI(EGL_OPENGL_ES_API);
        eglChooseConfig(eglDisplay, eglConfigAttribs, &eglConfig, 1, &n);
        eglContext = eglCreateContext(eglDisplay, eglConfig, EGL_NO_CONTEXT, eglContextAttribs);
        return true;
    }

    static bool initRenderThread()
    {
        LOutput::Params params
        {
            .callback = [](LOutput *output)
            {
                dummyOutputModes.push_back(new LOutputMode(output));
                output->imp()->updateRect();
            },
            .backendData = nullptr
        };

        shared.fd[2].fd = -1;
        shared.fd[0].fd = eventfd(0, O_CLOEXEC | O_NONBLOCK);
        shared.fd[0].events = POLLIN;
        shared.fd[0].revents = 0;

        dummyOutputs.push_back(Louvre::compositor()->createOutputRequest(&params));
        renderThread = std::thread(renderLoop);

        while (!windowInitialized)
            usleep(10000);

        return windowInitialized == 1;
    }

    static void createWindow()
    {
        windowEGLContext = eglCreateContext(eglDisplay, eglConfig, eglContext, eglContextAttribs);
        surface = wl_compositor_create_surface(compositor);
        wl_surface_add_listener(surface, &surfaceListener, nullptr);

        xdgSurface = xdg_wm_base_get_xdg_surface(xdgWmBase, surface);
        xdg_surface_add_listener(xdgSurface, &xdgSurfaceListener, nullptr);

        xdgToplevel = xdg_surface_get_toplevel(xdgSurface);
        xdg_toplevel_add_listener(xdgToplevel, &xdgToplevelListener, nullptr);

        if (xdgDecorationManager)
        {
            xdgDecoration = zxdg_decoration_manager_v1_get_toplevel_decoration(xdgDecorationManager, xdgToplevel);
            zxdg_toplevel_decoration_v1_set_mode(xdgDecoration, ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
        }

        wl_surface_attach(surface, NULL, 0, 0);
        wl_surface_commit(surface);
        wl_display_roundtrip(display);

        shared.surfaceSize = pendingSurfaceSize;
        shared.bufferScale = pendingBufferScale;
        shared.bufferSize = shared.surfaceSize * shared.bufferScale;
        eglWindow = wl_egl_window_create(surface, shared.bufferSize.w(), shared.bufferSize.h());
        eglSurface = eglCreateWindowSurface(eglDisplay, eglConfig,(EGLNativeWindowType) eglWindow, NULL);
        eglMakeCurrent(eglDisplay, eglSurface, eglSurface, windowEGLContext);
        wl_display_roundtrip(display);
    }

    static void renderLoop()
    {
        createWindow();
        LOutput *output { dummyOutputs.front() };
        output->imp()->updateRect();
        windowInitialized = 1;
        eventfd_t value;

        while (true)
        {
            while (wl_display_prepare_read(display) != 0)
                wl_display_dispatch_pending(display);
            wl_display_flush(display);

            poll(shared.fd, 2, -1);

            if (shared.fd[0].revents & POLLIN)
                eventfd_read(shared.fd[0].fd, &value);

            if (shared.fd[1].revents & POLLIN)
            {
                wl_display_read_events(display);
                unlockInputThread();
            }
            else
                wl_display_cancel_read(display);

            if (output->state() == LOutput::Initialized && repaint)
            {
                eglSwapInterval(eglDisplay, vSync);

                repaint = false;
                output->imp()->stateFlags.add(LOutput::LOutputPrivate::NeedsFullRepaint);

                if (wl_surface_get_version(surface) >= 3)
                    wl_surface_set_buffer_scale(surface, pendingBufferScale);

                output->setScale(pendingBufferScale);

                if (pendingSurfaceSize != shared.surfaceSize || pendingBufferScale != shared.bufferScale)
                {
                    shared.surfaceSize = pendingSurfaceSize;
                    shared.bufferScale = pendingBufferScale;
                    shared.bufferSize = shared.surfaceSize * shared.bufferScale;
                    output->imp()->updateRect();
                    wl_egl_window_resize(eglWindow,
                                         shared.bufferSize.w(),
                                         shared.bufferSize.h(), 0, 0);
                }

                output->imp()->backendPaintGL();

                eglSwapBuffers(eglDisplay, eglSurface);

                if (!vSync && refreshRateLimit >= 0)
                {
                    Int64 diff {LTime::us() - lastFrameUsec};
                    Int64 target;
                    if (refreshRateLimit == 0)
                        target = (1000000/((2 * refreshRate)/1000));
                    else
                        target = (1000000/refreshRateLimit);

                    target -= diff;

                    if (target > 0)
                        usleep(target);

                    lastFrameUsec = LTime::us();
                }

                output->imp()->presentationTime.flags = SRM_PRESENTATION_TIME_FLAGS_VSYNC;
                output->imp()->presentationTime.frame = 0;
                output->imp()->presentationTime.period = 0;
                clock_gettime(CLOCK_MONOTONIC, &output->imp()->presentationTime.time);
                output->imp()->backendPageFlipped();

            }
            else if (output->state() == LOutput::PendingInitialize)
                output->imp()->backendInitializeGL();
            else if (output->state() == LOutput::PendingUninitialize)
                output->imp()->backendUninitializeGL();

            wl_display_dispatch_pending(display);
        }
    }

    static bool backendInitialize()
    {
        Louvre::compositor()->imp()->graphicBackendData = &shared;

        if (!initWayland())
            goto fail;

        if (!initEGL())
            goto fail;

        if (!initRenderThread())
            goto fail;

        return true;

    fail:
        return false;
    }

    static void backendUninitialize()
    {
        /* TODO */
    }

    static void backendSuspend()
    {
        /* TODO */
    }

    static void backendResume()
    {
        /* TODO */
    }

    static const std::vector<LOutput*>* backendGetConnectedOutputs()
    {
        return &dummyOutputs;
    }

    static UInt32 backendGetRendererGPUs()
    {
        return 1;
    }

    static const std::vector<LDMAFormat>* backendGetDMAFormats()
    {
        static std::vector<LDMAFormat> dummyFormats;
        return &dummyFormats;
    }

    static EGLDisplay backendGetAllocatorEGLDisplay()
    {
        return eglDisplay;
    }

    static EGLContext backendGetAllocatorEGLContext()
    {
        return eglContext;
    }

    static dev_t backendGetAllocatorDeviceId()
    {
        return 0;
    }

    static bool textureCreateFromCPUBuffer(LTexture *texture, const LSize &size, UInt32 stride, UInt32 format, const void *pixels)
    {
        const SRMGLFormat *glFmt { srmFormatDRMToGL(format) };

        if (!glFmt)
            return false;

        UInt32 depth, bpp, pixelSize;

        if (!srmFormatGetDepthBpp(format, &depth, &bpp))
            return false;

        if (bpp % 8 != 0)
            return false;

        pixelSize = bpp/8;

        GLuint textureId { 0 };
        glGenTextures(1, &textureId);

        if (!textureId)
            return false;

        glBindTexture(GL_TEXTURE_2D, textureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        if (pixels)
        {

            glPixelStorei(GL_UNPACK_ROW_LENGTH_EXT, stride / pixelSize);
            glTexImage2D(GL_TEXTURE_2D,
                         0,
                         glFmt->glInternalFormat,
                         size.w(),
                         size.h(),
                         0,
                         glFmt->glFormat,
                         glFmt->glType,
                         pixels);
            glPixelStorei(GL_UNPACK_ROW_LENGTH_EXT, 0);
        }
        else
        {
            glTexImage2D(GL_TEXTURE_2D,
                         0,
                         glFmt->glInternalFormat,
                         size.w(),
                         size.h(),
                         0,
                         glFmt->glFormat,
                         glFmt->glType,
                         NULL);
        }

        glFlush();

        CPUTexture *cpuTexture { new CPUTexture() };
        cpuTexture->texture.id = textureId;
        cpuTexture->texture.target = GL_TEXTURE_2D;
        cpuTexture->glFmt = glFmt;
        cpuTexture->pixelSize = pixelSize;
        texture->m_graphicBackendData = cpuTexture;
        return true;
    }

    static bool textureCreateFromWaylandDRM(LTexture *texture,void *wlBuffer)
    {
        EGLint format, width, height;
        GLenum target { GL_TEXTURE_2D };
        EGLImage image;
        GLuint id;

        if (Louvre::compositor()->imp()->eglQueryWaylandBufferWL(LCompositor::eglDisplay(), (wl_resource*)wlBuffer, EGL_TEXTURE_FORMAT, &format))
        {
            Louvre::compositor()->imp()->eglQueryWaylandBufferWL(LCompositor::eglDisplay(), (wl_resource*)wlBuffer, EGL_WIDTH, &width);
            Louvre::compositor()->imp()->eglQueryWaylandBufferWL(LCompositor::eglDisplay(), (wl_resource*)wlBuffer, EGL_HEIGHT, &height);
            texture->m_sizeB.setW(width);
            texture->m_sizeB.setH(height);

            if (format == EGL_TEXTURE_RGB)
                texture->m_format = DRM_FORMAT_XRGB8888;
            else if (format == EGL_TEXTURE_RGBA)
                texture->m_format = DRM_FORMAT_ARGB8888;
            else if (format == EGL_TEXTURE_EXTERNAL_WL)
            {
                texture->m_format = DRM_FORMAT_YUYV;
                target = GL_TEXTURE_EXTERNAL_OES;
            }
            else
                texture->m_format = DRM_FORMAT_YUYV;

            EGLAttrib attribs = EGL_NONE;
            image = eglCreateImage(LCompositor::eglDisplay(), EGL_NO_CONTEXT, EGL_WAYLAND_BUFFER_WL, wlBuffer, &attribs);

            glGenTextures(1, &id);
            glBindTexture(target, id);
            glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            Louvre::compositor()->imp()->glEGLImageTargetTexture2DOES(target, image);

            DRMTexture *drmTexture { new DRMTexture() };
            drmTexture->texture.id = id;
            drmTexture->texture.target = target;
            drmTexture->image = image;
            texture->m_graphicBackendData = drmTexture;
            return true;
        }

        return false;
    }

    static bool textureCreateFromDMA(LTexture */*texture*/, const LDMAPlanes */*planes*/)
    {
        /* Not supported yet, wl_drm can be used instead */
        return false;
    }

    static bool textureUpdateRect(LTexture *texture, UInt32 stride, const LRect &dst, const void *pixels)
    {
        if (texture->sourceType() != LTexture::CPU)
            return false;

        CPUTexture *cpuTexture = (CPUTexture*)texture->m_graphicBackendData;

        if (!cpuTexture)
            return false;

        glBindTexture(GL_TEXTURE_2D, cpuTexture->texture.id);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, stride / cpuTexture->pixelSize);
        glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
        glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);

        glTexSubImage2D(GL_TEXTURE_2D, 0, dst.x(), dst.y(), dst.w(), dst.h(),
                        cpuTexture->glFmt->glFormat, cpuTexture->glFmt->glType, pixels);

        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
        glFlush();
        return true;
    }

    static UInt32 textureGetID(LOutput */*output*/, LTexture *texture)
    {
        Texture *bkndTexture { static_cast<Texture*>(texture->m_graphicBackendData) };

        if (bkndTexture)
            return bkndTexture->id;

        return 0;
    }

    static GLenum textureGetTarget(LTexture *texture)
    {
        Texture *bkndTexture = (Texture*)texture->m_graphicBackendData;

        if (bkndTexture)
            return bkndTexture->target;

        return GL_TEXTURE_2D;
    }

    static void textureDestroy(LTexture *texture)
    {
        if (texture->sourceType() == LTexture::CPU)
        {
            CPUTexture *cpuTexture = (CPUTexture*)texture->m_graphicBackendData;

            if (cpuTexture)
            {
                glDeleteTextures(1, &cpuTexture->texture.id);
                delete cpuTexture;
            }
        }
        else if (texture->sourceType() == LTexture::WL_DRM)
        {
            DRMTexture *drmTexture = (DRMTexture*)texture->m_graphicBackendData;

            if (drmTexture)
            {
                glDeleteTextures(1, &drmTexture->texture.id);
                eglDestroyImage(LCompositor::eglDisplay(), drmTexture->image);
                delete drmTexture;
            }
        }
    }

    /* OUTPUT */
    static bool outputInitialize(LOutput */*output*/)
    {
        /* TODO */
        return true;
    }

    static bool outputRepaint(LOutput */*output*/)
    {
        eventfd_write(shared.fd[0].fd, 1);
        repaint = true;
        return true;
    }

    static void outputUninitialize(LOutput */*output*/)
    {
        /* TODO */
    }

    static bool outputHasBufferDamageSupport(LOutput */*output*/)
    {
        return false;
    }

    static void outputSetBufferDamage(LOutput */*output*/, LRegion &/*region*/)
    {
        /* Disabled */
    }

    /* OUTPUT PROPS */
    static const char *outputGetName(LOutput */*output*/)
    {
        return "Wayland-EGL-1";
    }

    static const char *outputGetManufacturerName(LOutput */*output*/)
    {
        return "Cuarzo Software";
    }

    static const char *outputGetModelName(LOutput */*output*/)
    {
        return "Wayland EGL Window";
    }

    static const char *outputGetDescription(LOutput */*output*/)
    {
        return "Louvre compositor running on a Wayland EGL window";
    }

    static const LSize *outputGetPhysicalSize(LOutput */*output*/)
    {
        return &physicalSize;
    }

    static Int32 outputGetSubPixel(LOutput */*output*/)
    {
        // TODO: Get from current wl_output.
        return WL_OUTPUT_SUBPIXEL_UNKNOWN;
    }

    static Int32 outputGetCurrentBufferIndex(LOutput */*output*/)
    {
        return 0;
    }

    static UInt32 outputGetBuffersCount(LOutput */*output*/)
    {
        /* Damage tracking is disabled so we fake 1 */
        return 1;
    }

    static LTexture *outputGetBuffer(LOutput */*output*/, UInt32 /*bufferIndex*/)
    {
        return nullptr;
    }

    static UInt32 outputGetGammaSize(LOutput */*output*/)
    {
        return 0;
    }

    static bool outputSetGamma(LOutput */*output*/, const LGammaTable &/*table*/)
    {
        return false;
    }

    static bool outputHasVSyncControlSupport(LOutput */*output*/)
    {
        return true;
    }

    static bool outputIsVSyncEnabled(LOutput */*output*/)
    {
        return vSync;
    }

    static bool outputEnableVSync(LOutput */*output*/, bool enabled)
    {
        vSync = enabled;
        return true;
    }

    static void outputSetRefreshRateLimit(LOutput */*output*/, Int32 hz)
    {
        refreshRateLimit = hz;
    }

    static Int32 outputGetRefreshRateLimit(LOutput */*output*/)
    {
        return refreshRateLimit;
    }

    static clockid_t outputGetClock(LOutput */*output*/)
    {
        return CLOCK_MONOTONIC;
    }

    static bool outputHasHardwareCursorSupport(LOutput */*output*/)
    {
        return cursorMap != nullptr;
    }

    static void outputSetCursorTexture(LOutput */*output*/, UChar8 *buffer)
    {
        shared.mutex.lock();
        if (buffer)
        {
            memcpy(cursorMap, buffer, CURSOR_SIZE);
            wl_buffer_destroy(shared.cursorBuffer);
            LSize size { cursor()->rect().size() * shared.bufferScale };

            if (size.w() > 64)
                size.setW(64);

            if (size.h() > 64)
                size.setH(64);

            shared.cursorBuffer = wl_shm_pool_create_buffer(shmPool,
                                                            0,
                                                            size.w(),
                                                            size.h(),
                                                            64 * 4, WL_SHM_FORMAT_ARGB8888);
            wl_surface_attach(shared.cursorSurface, shared.cursorBuffer, 0, 0);
            shared.cursorVisible = true;
            shared.cursorChangedBuffer = true;
        }
        else
        {
            shared.cursorVisible = false;
            shared.cursorChangedBuffer = true;
        }

        unlockInputThread();
        shared.mutex.unlock();
    }

    static void outputSetCursorPosition(LOutput */*output*/, const LPoint &/*position*/)
    {
        static LPointF prevHotspotB;

        if (prevHotspotB != cursor()->hotspotB())
        {
            prevHotspotB = cursor()->hotspotB();
            shared.mutex.lock();
            shared.cursorHotspot = cursor()->hotspotB();
            shared.cursorChangedHotspot = true;
            unlockInputThread();
            shared.mutex.unlock();
        }
    }

    static const LOutputMode *outputGetPreferredMode(LOutput */*output*/)
    {
        return dummyOutputModes.front();
    }

    static const LOutputMode *outputGetCurrentMode(LOutput */*output*/)
    {
        return dummyOutputModes.front();
    }

    static const std::vector<LOutputMode*>* outputGetModes(LOutput */*output*/)
    {
        return &dummyOutputModes;
    }

    static bool outputSetMode(LOutput */*output*/, LOutputMode */*mode*/)
    {
        return true;
    }

    static const LSize *outputModeGetSize(LOutputMode */*mode*/)
    {
        return &shared.bufferSize;
    }

    static Int32 outputModeGetRefreshRate(LOutputMode */*mode*/)
    {
        return refreshRate;
    }

    static bool outputModeIsPreferred(LOutputMode */*mode*/)
    {
        return true;
    }

    static void registryHandleGlobal(void */*data*/, wl_registry *registry, UInt32 name, const char *interface, UInt32 version)
    {
        if (!compositor && strcmp(interface, wl_compositor_interface.name) == 0)
            compositor = (wl_compositor*)wl_registry_bind(registry, name, &wl_compositor_interface, version >= 3 ? 3 : 1);

        else if (!xdgWmBase && strcmp(interface, xdg_wm_base_interface.name) == 0)
        {
            xdgWmBase = (xdg_wm_base*)wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
            xdg_wm_base_add_listener(xdgWmBase, &xdgWmBaseListener, nullptr);
        }

        else if (!xdgDecorationManager && strcmp(interface, zxdg_decoration_manager_v1_interface.name) == 0)
        {
            xdgDecorationManager = (zxdg_decoration_manager_v1*)wl_registry_bind(registry, name, &zxdg_decoration_manager_v1_interface, 1);
        }

        else if (!shm && strcmp(interface, wl_shm_interface.name) == 0)
        {
            shm = (wl_shm*)wl_registry_bind(registry, name, &wl_shm_interface, 1);
        }

        else if (version >= 2 && strcmp(interface, wl_output_interface.name) == 0)
        {
            WaylandOutput *output { new WaylandOutput() };
            output->name = name;
            waylandOutputs.emplace_back((wl_output*)wl_registry_bind(registry, name, &wl_output_interface, 2));
            wl_output_add_listener(waylandOutputs.back(), &outputListener, output);
            wl_proxy_set_user_data((wl_proxy*)waylandOutputs.back(), output);
        }
    }

    static void registryHandleGlobalRemove(void */*data*/, wl_registry */*registry*/, UInt32 name)
    {
        WaylandOutput *waylandOutput;
        for (std::size_t i {0}; i < waylandOutputs.size(); i++)
        {
            waylandOutput = static_cast<WaylandOutput*>((wl_output_get_user_data(waylandOutputs[i])));

            if (waylandOutput->name == name)
            {
                LVectorRemoveOneUnordered(surfaceOutputs, waylandOutputs[i]);
                waylandOutputs[i] = waylandOutputs.back();
                waylandOutputs.pop_back();
                wl_proxy_destroy((wl_proxy*)waylandOutputs[i]);
                delete waylandOutput;
                updateSurfaceScale();
                return;
            }
        }
    }

    static void surfaceHandleEnter(void */*data*/, wl_surface *surface, wl_output *output)
    {
        if (surface == shared.cursorSurface)
            return;
        LVectorPushBackIfNonexistent(surfaceOutputs, output);
        updateSurfaceScale();
    }

    static void surfaceHandleLeave(void */*data*/, wl_surface */*surface*/, wl_output *output)
    {
        if (surface == shared.cursorSurface)
            return;
        LVectorRemoveOneUnordered(surfaceOutputs, output);
        updateSurfaceScale();
    }

    static void updateSurfaceScale()
    {
        const Int32 oldScale { pendingBufferScale };
        pendingBufferScale = 1;

        for (auto *output : surfaceOutputs)
        {
            WaylandOutput &outputData { *static_cast<WaylandOutput*>(wl_output_get_user_data(output)) };

            if (pendingBufferScale < outputData.bufferScale)
                pendingBufferScale = outputData.bufferScale;
        }

        if (pendingBufferScale != oldScale)
            outputRepaint(nullptr);
    }

    static void outputHandleMode(void *data, wl_output *, UInt32 /*flags*/, Int32 /*width*/, Int32 /*height*/, Int32 refresh)
    {
        WaylandOutput &output { *static_cast<WaylandOutput*>(data) };
        output.refresh = refresh;
    }

    static void outputHandleScale(void *data, wl_output *, Int32 scale)
    {
        WaylandOutput &output { *static_cast<WaylandOutput*>(data) };
        output.bufferScale = scale;
        updateSurfaceScale();
    }

    static void outputHandleDone(void *, wl_output *) {}

    static void xdgWmBaseHandlePing(void */*data*/, xdg_wm_base *wm_base, UInt32 serial)
    {
        xdg_wm_base_pong(wm_base, serial);
    }

    static void xdgSurfaceHandleConfigure(void *, xdg_surface *xdgSurface, UInt32 serial)
    {
        xdg_surface_ack_configure(xdgSurface, serial);
    }

    static void xdgToplevelHandleClose(void*, xdg_toplevel*)
    {
        Louvre::compositor()->finish();
    }

    static void xdgToplevelHandleConfigure(void*, xdg_toplevel*, Int32 w, Int32 h, wl_array*)
    {
        if (w > 0)
            pendingSurfaceSize.setW(w);

        if (h > 0)
            pendingSurfaceSize.setH(h);

        if (pendingSurfaceSize != shared.surfaceSize)
            outputRepaint(nullptr);
    }

};

extern "C" LGraphicBackendInterface *getAPI()
{
    static LGraphicBackendInterface API;
    API.backendGetId                    = &LGraphicBackend::backendGetId;
    API.backendGetContextHandle         = &LGraphicBackend::backendGetContextHandle;
    API.backendInitialize               = &LGraphicBackend::backendInitialize;
    API.backendUninitialize             = &LGraphicBackend::backendUninitialize;
    API.backendSuspend                  = &LGraphicBackend::backendSuspend;
    API.backendResume                   = &LGraphicBackend::backendResume;
    API.backendGetConnectedOutputs      = &LGraphicBackend::backendGetConnectedOutputs;
    API.backendGetRendererGPUs          = &LGraphicBackend::backendGetRendererGPUs;
    API.backendGetDMAFormats            = &LGraphicBackend::backendGetDMAFormats;
    API.backendGetAllocatorEGLDisplay   = &LGraphicBackend::backendGetAllocatorEGLDisplay;
    API.backendGetAllocatorEGLContext   = &LGraphicBackend::backendGetAllocatorEGLContext;
    API.backendGetAllocatorDeviceId     = &LGraphicBackend::backendGetAllocatorDeviceId;

    /* TEXTURES */
    API.textureCreateFromCPUBuffer      = &LGraphicBackend::textureCreateFromCPUBuffer;
    API.textureCreateFromWaylandDRM     = &LGraphicBackend::textureCreateFromWaylandDRM;
    API.textureCreateFromDMA            = &LGraphicBackend::textureCreateFromDMA;
    API.textureUpdateRect               = &LGraphicBackend::textureUpdateRect;
    API.textureGetID                    = &LGraphicBackend::textureGetID;
    API.textureGetTarget                = &LGraphicBackend::textureGetTarget;
    API.textureDestroy                  = &LGraphicBackend::textureDestroy;

    /* OUTPUT */
    API.outputInitialize                = &LGraphicBackend::outputInitialize;
    API.outputRepaint                   = &LGraphicBackend::outputRepaint;
    API.outputUninitialize              = &LGraphicBackend::outputUninitialize;
    API.outputHasBufferDamageSupport    = &LGraphicBackend::outputHasBufferDamageSupport;
    API.outputSetBufferDamage           = &LGraphicBackend::outputSetBufferDamage;

    /* OUTPUT PROPS */
    API.outputGetName                   = &LGraphicBackend::outputGetName;
    API.outputGetManufacturerName       = &LGraphicBackend::outputGetManufacturerName;
    API.outputGetModelName              = &LGraphicBackend::outputGetModelName;
    API.outputGetDescription            = &LGraphicBackend::outputGetDescription;
    API.outputGetPhysicalSize           = &LGraphicBackend::outputGetPhysicalSize;
    API.outputGetSubPixel               = &LGraphicBackend::outputGetSubPixel;

    /* OUTPUT BUFFERING */
    API.outputGetCurrentBufferIndex     = &LGraphicBackend::outputGetCurrentBufferIndex;
    API.outputGetBuffersCount           = &LGraphicBackend::outputGetBuffersCount;
    API.outputGetBuffer                 = &LGraphicBackend::outputGetBuffer;

    /* OUTPUT GAMMA */
    API.outputGetGammaSize              = &LGraphicBackend::outputGetGammaSize;
    API.outputSetGamma                  = &LGraphicBackend::outputSetGamma;

    /* OUTPUT V-SYNC */
    API.outputHasVSyncControlSupport    = &LGraphicBackend::outputHasVSyncControlSupport;
    API.outputIsVSyncEnabled            = &LGraphicBackend::outputIsVSyncEnabled;
    API.outputEnableVSync               = &LGraphicBackend::outputEnableVSync;
    API.outputSetRefreshRateLimit       = &LGraphicBackend::outputSetRefreshRateLimit;
    API.outputGetRefreshRateLimit       = &LGraphicBackend::outputGetRefreshRateLimit;

    /* OUTPUT TIME */
    API.outputGetClock                  = &LGraphicBackend::outputGetClock;

    /* OUTPUT CURSOR */
    API.outputHasHardwareCursorSupport  = &LGraphicBackend::outputHasHardwareCursorSupport;
    API.outputSetCursorTexture          = &LGraphicBackend::outputSetCursorTexture;
    API.outputSetCursorPosition         = &LGraphicBackend::outputSetCursorPosition;

    /* OUTPUT MODES */
    API.outputGetPreferredMode          = &LGraphicBackend::outputGetPreferredMode;
    API.outputGetCurrentMode            = &LGraphicBackend::outputGetCurrentMode;
    API.outputGetModes                  = &LGraphicBackend::outputGetModes;
    API.outputSetMode                   = &LGraphicBackend::outputSetMode;

    /* OUTPUT MODE PROPS */
    API.outputModeGetSize               = &LGraphicBackend::outputModeGetSize;
    API.outputModeGetRefreshRate        = &LGraphicBackend::outputModeGetRefreshRate;
    API.outputModeIsPreferred           = &LGraphicBackend::outputModeIsPreferred;

    return &API;
}
