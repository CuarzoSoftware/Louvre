#ifndef LCOMPOSITORPRIVATE_H
#define LCOMPOSITORPRIVATE_H

#include <LCompositor.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <sys/poll.h>

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
    wl_display *display = nullptr;
    wl_event_loop *eventLoop = nullptr;
    pollfd fdSet;
    wl_listener clientConnectedListener;
    wl_event_source *clientDisconnectedEventSource;
    PFNEGLBINDWAYLANDDISPLAYWL eglBindWaylandDisplayWL = NULL;
    EGLDisplay mainEGLDisplay = EGL_NO_DISPLAY;
    EGLContext mainEGLContext = EGL_NO_CONTEXT;
    bool initWayland();
    void unitWayland();

    LCursor *cursor = nullptr;
    LSeat *seat                                                 = nullptr;
    LSession *session                                           = nullptr;
    Int32 globalScale                                           = 1;
    bool isGraphicBackendInitialized                            = false;
    bool isInputBackendInitialized                              = false;


    std::thread::id threadId;
    mutex renderMutex;

    bool loadGraphicBackend(const char *path);
    bool loadInputBackend(const char *path);

    void raiseChildren(LSurface *surface);
    void insertSurfaceAfter(LSurface *prevSurface, LSurface *surfaceToInsert);
    void insertSurfaceBefore(LSurface *nextSurface, LSurface *surfaceToInsert);
    void updateGlobalScale();

    // Clients
    list<LClient*>clients;

    // Outputs
    list<LOutput*>outputs;

    // Surfaces
    list<LSurface*>surfaces;

    // Output Manager
    LOutputManager *outputManager                               = nullptr;



    int waylandFd;

    LPainter *painter;

    LGraphicBackendInterface *graphicBackend                    = nullptr;
    LInputBackendInterface *inputBackend                        = nullptr;

    // Dylib
    void *graphicBackendHandle                                  = nullptr;
    void *inputBackendHandle                                    = nullptr;
    void *graphicBackendData                                    = nullptr;

};


#endif // LCOMPOSITORPRIVATE_H
