#ifndef LCOMPOSITORPRIVATE_H
#define LCOMPOSITORPRIVATE_H

#include <private/LRenderBufferPrivate.h>
#include <LCompositor.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <sys/poll.h>
#include <map>
#include <unistd.h>

LPRIVATE_CLASS(LCompositor)

    /* We do not destroy globals immediatly as suggested here:
     * https://wayland.freedesktop.org/docs/html/apc.html#Server-wayland-server_8c_1a7f93649ba31c12220ee77982a37aa270
     * we wait some loop iterations (LOUVRE_GLOBAL_ITERS_BEFORE_DESTROY) before calling wl_global_destroy */
    struct RemovedGlobal
    {
        wl_global *global;
        UChar8 iters;
    };
    list<RemovedGlobal*>removedGlobals;
    void processRemovedGlobals();
    void removeGlobal(wl_global *global);

    CompositorState state = CompositorState::Uninitialized;
    LCompositor *compositor = nullptr;
    void uinitCompositor();

    bool initWayland();
        wl_display *display = nullptr;
        wl_event_loop *eventLoop = nullptr;
        pollfd fdSet[2];
        wl_listener clientConnectedListener;
        wl_event_source *clientDisconnectedEventSource;
    void unitWayland();

    bool initSeat();
        LSeat *seat = nullptr;
        LSession *session = nullptr;
    void unitSeat();

    bool initGraphicBackend();
        PFNEGLBINDWAYLANDDISPLAYWL eglBindWaylandDisplayWL = NULL;
        EGLDisplay mainEGLDisplay = EGL_NO_DISPLAY;
        EGLContext mainEGLContext = EGL_NO_CONTEXT;
        LGraphicBackendInterface *graphicBackend = nullptr;
        void *graphicBackendHandle = nullptr; // Dylib
        void *graphicBackendData = nullptr;
        LCursor *cursor = nullptr;
        LPainter *painter;
        bool isGraphicBackendInitialized = false;
    void unitGraphicBackend();

    bool isInputBackendInitialized                              = false;

    // Event FD to force unlock main loop poll
    bool pollUnlocked = false;
    void unlockPoll();

    // Threads sync
    std::thread::id threadId;
    mutex renderMutex;
    mutex queueMutex;
    std::list<std::thread::id>threadsQueue;
    void lock();
    void unlock();

    bool loadGraphicBackend(const char *path);
    bool loadInputBackend(const char *path);

    void raiseChildren(LSurface *surface);
    void insertSurfaceAfter(LSurface *prevSurface, LSurface *surfaceToInsert);
    void insertSurfaceBefore(LSurface *nextSurface, LSurface *surfaceToInsert);

    list<LClient*>clients;
    list<LOutput*>outputs;
    list<LSurface*>surfaces;
    list<LView*>views;
    list<LAnimation*>animations;

    bool runningAnimations();
    void processAnimations(bool updateOnly = false);

    LInputBackendInterface *inputBackend = nullptr;

    // Dylib
    void *inputBackendHandle = nullptr;

    // Thread specific data
    struct ThreadData
    {
        LPainter *painter = nullptr;
        std::list<LRenderBuffer::LRenderBufferPrivate::ThreadData> renderBuffersToDestroy;
    };

    std::map<std::thread::id, ThreadData> threadsMap;
    void destroyPendingRenderBuffers(std::thread::id *id);
    void addRenderBufferToDestroy(thread::id thread, LRenderBuffer::LRenderBufferPrivate::ThreadData &data);
};


#endif // LCOMPOSITORPRIVATE_H
