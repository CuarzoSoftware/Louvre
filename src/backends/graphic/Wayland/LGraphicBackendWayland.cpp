#include <xdg-decoration-unstable-v1-client.h>
#include <xdg-shell-client.h>
#include <wayland-client.h>
#include <wayland-egl.h>
#include <CZ/Louvre/Private/LCompositorPrivate.h>
#include <CZ/Louvre/Private/LOutputPrivate.h>
#include <CZ/Louvre/Private/LFactory.h>
#include <CZ/Louvre/LUtils.h>

#include <LOutputMode.h>
#include <SRMFormat.h>
#include <SRMEGL.h>
#include <LCursor.h>
#include <LGPU.h>
#include <LTime.h>
#include <LLog.h>

#include <EGL/eglext.h>
#include <sys/mman.h>
#include <cstring>
#include <fcntl.h>

#include "WaylandBackendShared.h"

using namespace Louvre;

#define BKND_NAME "WAYLAND BACKEND"

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
    bool destroy;
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
    inline static wl_region *opaqueRegion { nullptr };
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
    inline static EGLDisplay eglDisplay { EGL_NO_DISPLAY };
    inline static EGLContext eglContext { EGL_NO_CONTEXT };
    inline static EGLContext windowEGLContext { EGL_NO_CONTEXT };
    inline static PFNEGLSWAPBUFFERSWITHDAMAGEKHRPROC eglSwapBuffersWithDamageKHR { nullptr };
    inline static LRegion damage;
    inline static bool bufferIsLocked { false };
    inline static EGLConfig eglConfig { EGL_NO_CONFIG_KHR };
    inline static EGLSurface eglSurface { EGL_NO_SURFACE };
    inline static std::thread renderThread;
    inline static UInt32 refreshRate { 60000 };
    inline static Int32 refreshRateLimit { 0 };
    inline static LSize physicalSize { 0, 0 };
    inline static LSize pendingSurfaceSize { 1024, 512 };
    inline static Int32 pendingBufferScale { 1 };
    inline static std::vector<LGPU*> devices;
    inline static LGPU allocator;
    inline static std::vector<LOutput*> dummyOutputs;
    inline static std::vector<LOutputMode *> dummyOutputModes;
    inline static LOutputMode defaultMode { nullptr, LSize(), 0, true, nullptr};
    inline static Int8 windowInitialized { 0 };
    inline static Int64 lastFrameUsec { 0 };
    inline static bool repaint { false };
    inline static bool vSync { true };
    inline static LContentType contentType { LContentTypeNone };

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
        if (!shared.shm)
            return false;

        shared.cursorMapFd = Louvre::createSHM(LOUVRE_WAYLAND_BACKEND_CURSOR_SIZE * LOUVRE_WAYLAND_BACKEND_CURSORS);

        if (shared.cursorMapFd < 0)
            return false;

        shared.cursorMap = (UInt8*)mmap(NULL, LOUVRE_WAYLAND_BACKEND_CURSOR_SIZE * LOUVRE_WAYLAND_BACKEND_CURSORS, PROT_READ | PROT_WRITE, MAP_SHARED, shared.cursorMapFd, 0);

        if (shared.cursorMap == MAP_FAILED)
        {
            shared.cursorMap = nullptr;
            close(shared.cursorMapFd);
            return false;
        }

        shared.shmPool = wl_shm_create_pool(shared.shm, shared.cursorMapFd, LOUVRE_WAYLAND_BACKEND_CURSOR_SIZE * LOUVRE_WAYLAND_BACKEND_CURSORS);
        shared.cursors.reserve(LOUVRE_WAYLAND_BACKEND_CURSORS);

        for (std::size_t i = 0; i < LOUVRE_WAYLAND_BACKEND_CURSORS; i++)
            shared.cursors.emplace_back(std::make_unique<WaylandBackendShared::SHMCursor>(shared, i));

        shared.currentCursor = shared.getFreeCursor();
        shared.cursorSurface = wl_compositor_create_surface(compositor);
        wl_surface_attach(shared.cursorSurface, shared.currentCursor->buffer, 0, 0);
        wl_display_roundtrip(display);
        return true;
    }

    static void unitCursor()
    {
        if (shared.cursorSurface)
        {
            wl_surface_destroy(shared.cursorSurface);
            shared.cursorSurface = nullptr;
        }

        shared.cursors.clear();

        if (shared.shmPool)
        {
            wl_shm_pool_destroy(shared.shmPool);
            shared.shmPool = nullptr;
        }

        if (shared.cursorMap)
        {
            munmap(shared.cursorMap, LOUVRE_WAYLAND_BACKEND_CURSOR_SIZE * LOUVRE_WAYLAND_BACKEND_CURSORS);
            shared.cursorMap = nullptr;
        }

        if (shared.cursorMapFd >= 0)
        {
            close(shared.cursorMapFd);
            shared.cursorMapFd = -1;
        }

        if (shared.shm)
        {
            wl_shm_destroy(shared.shm);
            shared.shm = nullptr;
        }
    }

    static bool initWayland()
    {
        devices.push_back(&allocator);
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

    static void unitWayland()
    {
        unitCursor();
        shared.fd[1].fd = -1;

        if (xdgWmBase)
        {
            xdg_wm_base_destroy(xdgWmBase);
            xdgWmBase = nullptr;
        }

        if (compositor)
        {
            wl_compositor_destroy(compositor);
            compositor = nullptr;
        }

        if (registry)
        {
            wl_registry_destroy(registry);
            registry = nullptr;
        }

        if (display)
        {
            wl_display_disconnect(display);
            display = 0;
        }
    }

    static bool initEGL()
    {
        EGLint major, minor, n;
        const char *extensions;
        eglDisplay = eglGetDisplay(display);

        if (eglDisplay == EGL_NO_DISPLAY)
        {
            LLog::fatal("[%s] Failed to get EGL display.", BKND_NAME);
            return false;
        }

        if (!eglInitialize(eglDisplay, &major, &minor))
        {
            LLog::fatal("[%s] Failed to initialize EGL display.", BKND_NAME);
            goto errDisplay;
        }

        if (!eglBindAPI(EGL_OPENGL_ES_API))
        {
            LLog::fatal("[%s] Failed to bind OpenGL ES API.", BKND_NAME);
            goto errTerminate;
        }

        if (!eglChooseConfig(eglDisplay, eglConfigAttribs, &eglConfig, 1, &n) || n != 1)
        {
            LLog::fatal("[%s] Failed to get EGL config.", BKND_NAME);
            goto errTerminate;
        }

        eglContext = eglCreateContext(eglDisplay, eglConfig, EGL_NO_CONTEXT, eglContextAttribs);

        if (eglContext == EGL_NO_CONTEXT)
        {
            LLog::fatal("[%s] Failed to get EGL context.", BKND_NAME);
            goto errTerminate;
        }

        extensions = eglQueryString(eglDisplay, EGL_EXTENSIONS);

        if (extensions && srmEGLHasExtension(extensions, "EGL_KHR_swap_buffers_with_damage"))
            eglSwapBuffersWithDamageKHR = (PFNEGLSWAPBUFFERSWITHDAMAGEKHRPROC)eglGetProcAddress("eglSwapBuffersWithDamageKHR");

        return true;

    errTerminate:
        eglTerminate(eglDisplay);
    errDisplay:
        eglDisplay = EGL_NO_DISPLAY;
        return false;
    }

    static void unitEGL()
    {
        if (eglContext != EGL_NO_CONTEXT)
        {
            eglDestroyContext(eglDisplay, eglContext);
            eglContext = EGL_NO_CONTEXT;
        }

        if (eglDisplay != EGL_NO_DISPLAY)
        {
            eglTerminate(eglDisplay);
            eglDisplay = EGL_NO_DISPLAY;
        }
    }

    static bool initRenderThread()
    {

        LOutput::Params params
        {
            .callback = [](LOutput *output)
            {
                defaultMode.m_output = output;
                defaultMode.m_refreshRate = refreshRate;
                defaultMode.m_sizeB = shared.bufferSize;
                dummyOutputModes.push_back(&defaultMode);
                output->imp()->updateRect();
            },
            .backendData = nullptr
        };

        shared.fd[2].fd = -1;
        shared.fd[0].fd = eventfd(0, O_CLOEXEC | O_NONBLOCK);
        shared.fd[0].events = POLLIN;
        shared.fd[0].revents = 0;

        dummyOutputs.push_back(LFactory::createObject<LOutput>(&params));
        renderThread = std::thread(renderLoop);

        while (!windowInitialized)
            usleep(10000);

        return windowInitialized == 1;
    }

    static void unitRenderThread()
    {
        windowInitialized = 0;

        eventfd_write(shared.fd[0].fd, 1);
        renderThread.join();

        close(shared.fd[0].fd);
        shared.fd[0].fd = -1;

        seat()->outputUnplugged(dummyOutputs.front());
        Louvre::compositor()->onAnticipatedObjectDestruction(dummyOutputs.front());
        delete dummyOutputs.front();
        dummyOutputs.clear();
        dummyOutputModes.clear();
    }

    static void createWindow()
    {
        windowEGLContext = eglCreateContext(eglDisplay, eglConfig, eglContext, eglContextAttribs);
        surface = wl_compositor_create_surface(compositor);
        wl_surface_add_listener(surface, &surfaceListener, nullptr);

        opaqueRegion = wl_compositor_create_region(compositor);
        wl_region_add(opaqueRegion,
            0, 0,
            std::numeric_limits<std::int32_t>::max(),
            std::numeric_limits<std::int32_t>::max());

        xdgSurface = xdg_wm_base_get_xdg_surface(xdgWmBase, surface);
        xdg_surface_add_listener(xdgSurface, &xdgSurfaceListener, nullptr);

        xdgToplevel = xdg_surface_get_toplevel(xdgSurface);
        xdg_toplevel_add_listener(xdgToplevel, &xdgToplevelListener, nullptr);
        xdg_toplevel_set_app_id(xdgToplevel, "com.CuarzoSoftware.Louvre");
        xdg_toplevel_set_title(xdgToplevel, "Wayland-EGL-1");

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
        defaultMode.m_sizeB = shared.bufferSize;
        eglWindow = wl_egl_window_create(surface, shared.bufferSize.w(), shared.bufferSize.h());
        eglSurface = eglCreateWindowSurface(eglDisplay, eglConfig,(EGLNativeWindowType) eglWindow, NULL);
        eglMakeCurrent(eglDisplay, eglSurface, eglSurface, windowEGLContext);
        wl_display_roundtrip(display);
    }

    static void destroyWindow()
    {
        eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

        if (eglSurface != EGL_NO_SURFACE)
        {
            eglDestroySurface(eglDisplay, eglSurface);
            eglSurface = EGL_NO_SURFACE;
        }

        if (eglWindow)
        {
            wl_egl_window_destroy(eglWindow);
            eglWindow = nullptr;
        }

        if (opaqueRegion)
        {
            wl_region_destroy(opaqueRegion);
            opaqueRegion = nullptr;
        }

        if (surface)
        {
            wl_surface_destroy(surface);
            surface = nullptr;
        }

        if (xdgDecoration)
        {
            zxdg_toplevel_decoration_v1_destroy(xdgDecoration);
            xdgDecoration = nullptr;
        }

        if (xdgDecorationManager)
        {
            zxdg_decoration_manager_v1_destroy(xdgDecorationManager);
            xdgDecorationManager = nullptr;
        }

        if (xdgToplevel)
        {
            xdg_toplevel_destroy(xdgToplevel);
            xdgToplevel = nullptr;
        }

        if (xdgSurface)
        {
            xdg_surface_destroy(xdgSurface);
            xdgSurface = nullptr;
        }

        if (surface)
        {
            wl_surface_destroy(surface);
            surface = nullptr;
        }

        if (windowEGLContext != EGL_NO_CONTEXT)
        {
            eglDestroyContext(eglDisplay, windowEGLContext);
            windowEGLContext = EGL_NO_CONTEXT;
        }
    }

    static void renderLoop()
    {
        createWindow();
        LOutput *output { dummyOutputs.front() };
        output->imp()->updateRect();
        windowInitialized = 1;
        eventfd_t value;

        while (windowInitialized == 1)
        {
            while (wl_display_prepare_read(display) != 0)
                wl_display_dispatch_pending(display);

            wl_display_flush(display);

            poll(shared.fd, 2, output->state() == LOutput::PendingInitialize ? 100 : -1);

            if (shared.fd[0].revents & POLLIN)
                eventfd_read(shared.fd[0].fd, &value);

            if (shared.fd[1].revents & POLLIN)
            {
                wl_display_read_events(display);
                unlockInputThread();
            }
            else
                wl_display_cancel_read(display);

        paintGLChangedBufferIsLocked:

            if (bufferIsLocked)
                repaint = false;

            if (output->state() == LOutput::Initialized && repaint)
            {
                repaint = false;

                eglSwapInterval(eglDisplay, vSync);

                if (wl_surface_get_version(surface) >= 3)
                    wl_surface_set_buffer_scale(surface, pendingBufferScale);

                output->setScale(pendingBufferScale);

                if (pendingSurfaceSize != shared.surfaceSize || pendingBufferScale != shared.bufferScale)
                {
                    shared.surfaceSize = pendingSurfaceSize;
                    shared.bufferScale = pendingBufferScale;
                    shared.bufferSize = shared.surfaceSize * shared.bufferScale;
                    defaultMode.m_sizeB = shared.bufferSize;
                    output->imp()->updateRect();
                    wl_egl_window_resize(eglWindow,
                                         shared.bufferSize.w(),
                                         shared.bufferSize.h(), 0, 0);
                }

                output->imp()->backendPaintGL();

                if (bufferIsLocked)
                    goto paintGLChangedBufferIsLocked;

                wl_surface_set_opaque_region(surface, opaqueRegion);

                if (eglSwapBuffersWithDamageKHR)
                {
                    if (damage.empty())
                    {
                        constexpr GLint dummyBox[4] { -100, -100, 1, 1 };
                        eglSwapBuffersWithDamageKHR(eglDisplay, eglSurface, dummyBox, 1);
                    }
                    else
                    {
                        Int32 n;
                        const LBox *boxes { damage.boxes(&n) };
                        GLint *rects = new GLint[n * 4];
                        GLint *rectsIt = rects;
                        for (Int32 i = 0; i < n; i++)
                        {
                            *rectsIt = boxes[i].x1;
                            rectsIt++;
                            *rectsIt = shared.bufferSize.h() - boxes[i].y2;
                            rectsIt++;
                            *rectsIt = boxes[i].x2 - boxes[i].x1;
                            rectsIt++;
                            *rectsIt = boxes[i].y2 - boxes[i].y1;
                            rectsIt++;
                        }
                        eglSwapBuffersWithDamageKHR(eglDisplay, eglSurface, rects, n);
                        delete []rects;
                    }
                }
                else
                {
                    if (damage.empty())
                        wl_surface_commit(surface);
                    else
                        eglSwapBuffers(eglDisplay, eglSurface);
                }

                damage.clear();

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

        destroyWindow();
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
        unitRenderThread();
        unitEGL();
        unitWayland();
        devices.clear();
        Louvre::compositor()->imp()->graphicBackendData = nullptr;
    }

    static void backendSuspend()
    {
        /* No TTY switching so no required */
    }

    static void backendResume()
    {
        /* No TTY switching so no required */
    }

    static const std::vector<LOutput*>* backendGetConnectedOutputs()
    {
        return &dummyOutputs;
    }

    static const std::vector<LGPU*>* backendGetDevices()
    {
        return &devices;
    }

    static const std::vector<LDMAFormat>* backendGetDMAFormats()
    {
        static std::vector<LDMAFormat> dummyFormats;
        return &dummyFormats;
    }

    static const std::vector<LDMAFormat> *backendGetScanoutDMAFormats()
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

    static LGPU *backendGetAllocatorDevice()
    {
        return &allocator;
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
        cpuTexture->destroy = true;
        texture->m_graphicBackendData = cpuTexture;
        return true;
    }

    static bool textureCreateFromWaylandDRM(LTexture *texture, void *wlBuffer)
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

            const static EGLAttrib attribs[3] {
                EGL_IMAGE_PRESERVED_KHR,
                EGL_TRUE,
                EGL_NONE
            };

            image = eglCreateImage(LCompositor::eglDisplay(), EGL_NO_CONTEXT, EGL_WAYLAND_BUFFER_WL, wlBuffer, attribs);

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

    static bool textureCreateFromGL(LTexture *texture, GLuint id, GLenum target, UInt32 format, const LSize &/*size*/, bool transferOwnership)
    {
        const SRMGLFormat *glFmt { srmFormatDRMToGL(format) };

        if (!glFmt)
            return false;

        UInt32 depth, bpp;

        if (!srmFormatGetDepthBpp(format, &depth, &bpp))
            return false;

        if (bpp % 8 != 0)
            return false;

        CPUTexture *cpuTexture { new CPUTexture() };
        cpuTexture->texture.id = id;
        cpuTexture->texture.target = target;
        cpuTexture->glFmt = glFmt;
        cpuTexture->pixelSize = bpp/8;
        cpuTexture->destroy = transferOwnership;
        texture->m_graphicBackendData = cpuTexture;
        return true;
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
        glFinish();
        return true;
    }

    static bool textureWriteBegin(LTexture *texture)
    {
        if (texture->sourceType() != LTexture::CPU)
            return false;

        CPUTexture *cpuTexture = (CPUTexture*)texture->m_graphicBackendData;

        if (!cpuTexture)
            return false;

        return true;
    }

    static bool textureWriteUpdate(LTexture *texture, UInt32 stride, const LRect &dst, const void *pixels)
    {
        CPUTexture *cpuTexture = (CPUTexture*)texture->m_graphicBackendData;
        glBindTexture(GL_TEXTURE_2D, cpuTexture->texture.id);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, stride / cpuTexture->pixelSize);
        glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
        glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);

        glTexSubImage2D(GL_TEXTURE_2D, 0, dst.x(), dst.y(), dst.w(), dst.h(),
                        cpuTexture->glFmt->glFormat, cpuTexture->glFmt->glType, pixels);
        return true;
    }

    static bool textureWriteEnd(LTexture */*texture*/)
    {
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
        glFinish();
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

    static void textureSetFence(LTexture */*texture*/)
    {
        /* TODO: Use fence */
        glFlush();
    }

    static void textureDestroy(LTexture *texture)
    {
        switch (texture->sourceType())
        {
        case LTexture::CPU:
        case LTexture::Framebuffer:
        case LTexture::GL:
            {
                CPUTexture *cpuTexture = (CPUTexture*)texture->m_graphicBackendData;

                if (cpuTexture)
                {
                    if (cpuTexture->destroy)
                        glDeleteTextures(1, &cpuTexture->texture.id);
                    delete cpuTexture;
                }
            }
            break;
        case LTexture::WL_DRM:
            {
                DRMTexture *drmTexture = (DRMTexture*)texture->m_graphicBackendData;

                if (drmTexture)
                {
                    glDeleteTextures(1, &drmTexture->texture.id);
                    eglDestroyImage(LCompositor::eglDisplay(), drmTexture->image);
                    delete drmTexture;
                }
            }
            break;
        case LTexture::DMA:
            break;
        }
    }

    /* OUTPUT */
    static bool outputInitialize(LOutput */*output*/)
    {
        eventfd_write(shared.fd[0].fd, 1);
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
        /* TODO: Should the compositor simply quit? */
    }

    static bool outputHasBufferDamageSupport(LOutput */*output*/)
    {
        return true;
    }

    static void outputSetBufferDamage(LOutput */*output*/, LRegion &region)
    {
        damage = region;
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

    static const char *outputGetSerial(LOutput */*output*/)
    {
        return nullptr;
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

    static LGPU *outputGetDevice(LOutput */*output*/)
    {
        return &allocator;
    }

    static UInt32 outputGetID(LOutput */*output*/)
    {
        return 0;
    }

    static bool outputIsNonDesktop(LOutput */*output*/)
    {
        return false;
    }

    static UInt32 outputGetFramebufferID(LOutput */*output*/)
    {
        return 0;
    }

    static Int32 outputGetCurrentBufferIndex(LOutput */*output*/)
    {
        return 0;
    }

    static UInt32 outputGetCurrentBufferAge(LOutput */*output*/)
    {
        GLint age { 0 };
        if (eglQuerySurface(eglDisplay, eglSurface, EGL_BUFFER_AGE_KHR, &age) == EGL_TRUE)
            return age;
        return 0;
    }

    static UInt32 outputGetBuffersCount(LOutput */*output*/)
    {
        /* Variable, so fake 1 */
        return 1;
    }

    static LTexture *outputGetBuffer(LOutput */*output*/, UInt32 /*bufferIndex*/)
    {
        return nullptr;
    }

    static void outputLockCurrentBuffer(LOutput */*output*/, bool locked)
    {
        bufferIsLocked = locked;
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
        return shared.cursorMap != nullptr;
    }

    static void outputSetCursorTexture(LOutput */*output*/, UInt8 *buffer)
    {
        shared.mutex.lock();

        if (buffer)
        {
            shared.currentCursor = shared.getFreeCursor();

            if (shared.currentCursor)
            {
                memcpy(shared.currentCursor->map, buffer, LOUVRE_WAYLAND_BACKEND_CURSOR_SIZE);
                wl_surface_attach(shared.cursorSurface, shared.currentCursor->buffer, 0, 0);
            }
            shared.cursorVisible = true;
            shared.cursorChangedBuffer = true;
        }
        else
        {
            shared.currentCursor = nullptr;
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
            shared.cursorHotspot = ((cursor()->pos() - cursor()->rect().pos()) * shared.bufferScale)/2;
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

    static LContentType outputGetContentType(LOutput */*output*/)
    {
        return contentType;
    }

    static void outputSetContentType(LOutput */*output*/, LContentType type)
    {
        contentType = type;
    }

    static bool outputSetScanoutBuffer(LOutput */*output*/, LTexture */*texture*/)
    {
        return false;
    }

    /* DRM LEASE */

    static int backendCreateLease(const std::vector<LOutput*> &/*outputs*/) { return -1; }
    static void backendRevokeLease(int /*fd*/) {}

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

        else if (!shared.shm && strcmp(interface, wl_shm_interface.name) == 0)
        {
            shared.shm = (wl_shm*)wl_registry_bind(registry, name, &wl_shm_interface, 1);
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
                wl_proxy_destroy((wl_proxy*)waylandOutputs[i]);
                waylandOutputs[i] = waylandOutputs.back();
                waylandOutputs.pop_back();
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
        {
            shared.cursorChangedBuffer = true;
            outputRepaint(nullptr);
        }
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
    API.backendGetDevices               = &LGraphicBackend::backendGetDevices;
    API.backendGetDMAFormats            = &LGraphicBackend::backendGetDMAFormats;
    API.backendGetScanoutDMAFormats     = &LGraphicBackend::backendGetScanoutDMAFormats;
    API.backendGetAllocatorEGLDisplay   = &LGraphicBackend::backendGetAllocatorEGLDisplay;
    API.backendGetAllocatorEGLContext   = &LGraphicBackend::backendGetAllocatorEGLContext;
    API.backendGetAllocatorDevice       = &LGraphicBackend::backendGetAllocatorDevice;

    /* TEXTURES */
    API.textureCreateFromCPUBuffer      = &LGraphicBackend::textureCreateFromCPUBuffer;
    API.textureCreateFromWaylandDRM     = &LGraphicBackend::textureCreateFromWaylandDRM;
    API.textureCreateFromDMA            = &LGraphicBackend::textureCreateFromDMA;
    API.textureCreateFromGL             = &LGraphicBackend::textureCreateFromGL;
    API.textureUpdateRect               = &LGraphicBackend::textureUpdateRect;
    API.textureWriteBegin               = &LGraphicBackend::textureWriteBegin;
    API.textureWriteUpdate              = &LGraphicBackend::textureWriteUpdate;
    API.textureWriteEnd                 = &LGraphicBackend::textureWriteEnd;
    API.textureGetID                    = &LGraphicBackend::textureGetID;
    API.textureGetTarget                = &LGraphicBackend::textureGetTarget;
    API.textureSetFence                 = &LGraphicBackend::textureSetFence;
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
    API.outputGetSerial                 = &LGraphicBackend::outputGetSerial;
    API.outputGetPhysicalSize           = &LGraphicBackend::outputGetPhysicalSize;
    API.outputGetSubPixel               = &LGraphicBackend::outputGetSubPixel;
    API.outputGetDevice                 = &LGraphicBackend::outputGetDevice;
    API.outputGetID                     = &LGraphicBackend::outputGetID;
    API.outputIsNonDesktop              = &LGraphicBackend::outputIsNonDesktop;

    /* OUTPUT BUFFERING */
    API.outputGetFramebufferID          = &LGraphicBackend::outputGetFramebufferID;
    API.outputGetCurrentBufferIndex     = &LGraphicBackend::outputGetCurrentBufferIndex;
    API.outputGetBuffersCount           = &LGraphicBackend::outputGetBuffersCount;
    API.outputGetCurrentBufferAge       = &LGraphicBackend::outputGetCurrentBufferAge;
    API.outputGetBuffer                 = &LGraphicBackend::outputGetBuffer;
    API.outputLockCurrentBuffer         = &LGraphicBackend::outputLockCurrentBuffer;

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

    /* CONTENT TYPE */
    API.outputGetContentType            = &LGraphicBackend::outputGetContentType;
    API.outputSetContentType            = &LGraphicBackend::outputSetContentType;

    /* DIRECT SCANOUT */
    API.outputSetScanoutBuffer          = &LGraphicBackend::outputSetScanoutBuffer;

    /* DRM LEASE */
    API.backendCreateLease              = &LGraphicBackend::backendCreateLease;
    API.backendRevokeLease              = &LGraphicBackend::backendRevokeLease;
    return &API;
}
