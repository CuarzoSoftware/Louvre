#ifndef LCOMPOSITORPRIVATE_H
#define LCOMPOSITORPRIVATE_H

#include <LOutput.h>
#include <private/LRenderBufferPrivate.h>
#include <LCompositor.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <sys/epoll.h>
#include <map>
#include <unistd.h>

LPRIVATE_CLASS(LCompositor)

    LVersion version;
    std::string defaultAssetsPath;
    std::string defaultBackendsPath;
    std::string defaultGraphicBackendName;
    std::string defaultInputBackendName;

    /* We do not destroy globals immediatly as suggested here:
     * https://wayland.freedesktop.org/docs/html/apc.html#Server-wayland-server_8c_1a7f93649ba31c12220ee77982a37aa270
     * we wait some loop iterations (LOUVRE_GLOBAL_ITERS_BEFORE_DESTROY) before calling wl_global_destroy */
    struct RemovedGlobal
    {
        wl_global *global;
        UChar8 iters;
    };
    std::list<RemovedGlobal*>removedGlobals;
    void processRemovedGlobals();
    void removeGlobal(wl_global *global);

    CompositorState state = CompositorState::Uninitialized;
    LCompositor *compositor = nullptr;
    void unitCompositor();

    bool initWayland();
        wl_display *display = nullptr;
        wl_event_loop *eventLoop = nullptr;
        epoll_event events[3];
        Int32 epollFd;
        wl_listener clientConnectedListener;
        wl_event_source *clientDisconnectedEventSource;
    void unitWayland();

    bool initSeat();
        LSeat *seat = nullptr;
        LSession *session = nullptr;
    void unitSeat();

    bool initGraphicBackend();
        PFNEGLBINDWAYLANDDISPLAYWL eglBindWaylandDisplayWL = NULL;
        PFNEGLQUERYWAYLANDBUFFERWL eglQueryWaylandBufferWL = NULL;
        EGLDisplay mainEGLDisplay = EGL_NO_DISPLAY;
        EGLContext mainEGLContext = EGL_NO_CONTEXT;
        LGraphicBackendInterface *graphicBackend = nullptr;
        void *graphicBackendHandle = nullptr; // Dylib
        void *graphicBackendData = nullptr;
        LCursor *cursor = nullptr;
        LPainter *painter;
        bool isGraphicBackendInitialized = false;
    void unitGraphicBackend(bool closeLib);

    bool initInputBackend();
        void *inputBackendHandle = nullptr;
        LInputBackendInterface *inputBackend = nullptr;
        bool isInputBackendInitialized = false;
    void unitInputBackend(bool closeLib);

    // Event FD to force unlock main loop poll
    bool pollUnlocked = false;
    void unlockPoll();

    // Threads sync
    std::thread::id threadId;
    std::mutex renderMutex;

    void lock();
    void unlock();

    bool loadGraphicBackend(const char *path);
    bool loadInputBackend(const char *path);

    void raiseChildren(LSurface *surface);
    void insertSurfaceAfter(LSurface *prevSurface, LSurface *surfaceToInsert);
    void insertSurfaceBefore(LSurface *nextSurface, LSurface *surfaceToInsert);

    std::list<LClient*>clients;
    std::list<LOutput*>outputs;
    std::list<LView*>views;
    std::list<LAnimation*>animations;
    std::list<LTexture*>textures;
    std::list<LSurface*>surfaces;
    bool surfacesListChanged = false;

    bool runningAnimations();
    void processAnimations();

    // Thread specific data
    struct ThreadData
    {
        LPainter *painter = nullptr;
        std::list<LRenderBuffer::LRenderBufferPrivate::ThreadData> renderBuffersToDestroy;
    };

    std::map<std::thread::id, ThreadData> threadsMap;
    void destroyPendingRenderBuffers(std::thread::id *id);
    void addRenderBufferToDestroy(std::thread::id thread, LRenderBuffer::LRenderBufferPrivate::ThreadData &data);
    static LPainter *findPainter();

    std::list<GLuint>nativeTexturesToDestroy;
    static void destroyNativeTextures(std::list<GLuint>&list);

    Int32 greatestOutputScale = 1;

    inline void updateGreatestOutputScale()
    {
        greatestOutputScale = 1;
        for (LOutput *o : outputs)
        {
            if (o->scale() > greatestOutputScale)
                greatestOutputScale = o->scale();
        }
    }
};

#endif // LCOMPOSITORPRIVATE_H
