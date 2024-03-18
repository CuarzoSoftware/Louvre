#ifndef LCOMPOSITORPRIVATE_H
#define LCOMPOSITORPRIVATE_H

#include <private/LBackendPrivate.h>
#include <LCompositor.h>
#include <LOutput.h>
#include <LInputDevice.h>
#include <LRenderBuffer.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <sys/epoll.h>
#include <map>
#include <unistd.h>
#include <string>
#include <filesystem>

using namespace Louvre;

LPRIVATE_CLASS(LCompositor)

    CompositorState state { CompositorState::Uninitialized };
    Int32 epollFd;
    LCompositor *compositor;
    LVersion version;
    std::filesystem::path defaultAssetsPath;
    std::filesystem::path defaultBackendsPath;
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
    std::vector<RemovedGlobal>removedGlobals;
    void processRemovedGlobals();
    void removeGlobal(wl_global *global);
    void unitCompositor();

    bool initWayland();
        wl_display *display { nullptr };
        wl_event_loop *eventLoop { nullptr };
        wl_listener clientConnectedListener;
        wl_event_source *clientDisconnectedEventSource;
        epoll_event events[3];
    void unitWayland();

    bool surfacesListChanged { false };
    bool animationsVectorChanged { false };
    bool pollUnlocked { false };
    bool isGraphicBackendInitialized { false };
    std::vector<GLuint>nativeTexturesToDestroy;

    bool initGraphicBackend();
        PFNEGLBINDWAYLANDDISPLAYWL eglBindWaylandDisplayWL { NULL };
        PFNEGLQUERYWAYLANDBUFFERWL eglQueryWaylandBufferWL { NULL };
        EGLDisplay mainEGLDisplay { EGL_NO_DISPLAY };
        EGLContext mainEGLContext { EGL_NO_CONTEXT };
        LGraphicBackendInterface *graphicBackend { nullptr };
        void *graphicBackendHandle { nullptr };
        void *graphicBackendData { nullptr };
        LCursor *cursor { nullptr };
        LPainter *painter { nullptr };
        LOutput *currentOutput { nullptr };
    void unitGraphicBackend(bool closeLib);

    bool initSeat();
        LSeat *seat { nullptr };
    void unitSeat();

    bool initInputBackend();
        LInputDevice fakeDevice { LSeat::Pointer | LSeat::Keyboard | LSeat::Touch };
        void *inputBackendHandle { nullptr };
        LInputBackendInterface *inputBackend { nullptr };
    void unitInputBackend(bool closeLib);

    // Event FD to force unlock main loop poll
    void unlockPoll();

    // Threads sync
    std::thread::id threadId;
    std::mutex renderMutex;

    void lock();
    void unlock();

    bool loadGraphicBackend(const std::filesystem::path &path);
    bool loadInputBackend(const std::filesystem::path &path);

    void raiseChildren(LSurface *surface);
    void insertSurfaceAfter(LSurface *prevSurface, LSurface *surfaceToInsert);
    void insertSurfaceBefore(LSurface *nextSurface, LSurface *surfaceToInsert);

    std::list<LSurface*>surfaces;
    std::vector<LClient*>clients;
    std::vector<LOutput*>outputs;
    std::vector<LView*>views;
    std::vector<LTexture*>textures;
    std::vector<LAnimation*>animations;
    std::vector<LTimer*>oneShotTimers;

    bool runningAnimations();
    void processAnimations();

    // Thread specific data
    struct ThreadData
    {
        LPainter *painter { nullptr };
        std::vector<LRenderBuffer::ThreadData> renderBuffersToDestroy;
    };

    std::map<std::thread::id, ThreadData> threadsMap;
    void destroyPendingRenderBuffers(std::thread::id *id);
    void addRenderBufferToDestroy(std::thread::id thread, LRenderBuffer::ThreadData &data);
    static LPainter *findPainter();

    inline static void destroyNativeTextures(std::vector<GLuint> &vector)
    {
        while (!vector.empty())
        {
            glDeleteTextures(1, &vector.back());
            vector.pop_back();
        }
    }

    void sendPendingToplevelsConfiguration();
    void sendPresentationTime();
    bool isInputBackendInitialized { false };
};

#endif // LCOMPOSITORPRIVATE_H
