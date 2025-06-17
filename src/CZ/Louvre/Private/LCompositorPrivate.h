#ifndef LCOMPOSITORPRIVATE_H
#define LCOMPOSITORPRIVATE_H

#include <CZ/Louvre/Private/LBackendPrivate.h>
#include <CZ/Louvre/LCompositor.h>
#include <CZ/Louvre/LOutput.h>
#include <CZ/Louvre/LPainter.h>
#include <CZ/Louvre/LInputDevice.h>
#include <CZ/Louvre/LRenderBuffer.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <sys/epoll.h>
#include <unordered_map>
#include <unistd.h>
#include <string>
#include <filesystem>
#include <set>
#include <unordered_set>

using namespace Louvre;

LPRIVATE_CLASS(LCompositor)

    CompositorState state { CompositorState::Uninitialized };
    LVersion version;
    std::filesystem::path defaultAssetsPath;
    std::filesystem::path defaultBackendsPath;
    std::string defaultGraphicBackendName;
    std::string defaultInputBackendName;

    std::vector<LGlobal*> globals;
    void processRemovedGlobals();
    void unitCompositor();

    bool initWayland();
        wl_display *display { nullptr };
        wl_event_loop *eventLoop { nullptr };
        Int32 eventLoopFd { -1 };
        wl_listener clientConnectedListener;
        wl_event_source *eventFdEventSource { nullptr };
        Int32 eventFd { -1 };
        LSessionLockManager *sessionLockManager { nullptr };
        LActivationTokenManager *activationTokenManager { nullptr };
    void unitWayland();

    bool surfacesListChanged { false };
    bool animationsVectorChanged { false };
    bool pollUnlocked { false };
    bool isGraphicBackendInitialized { false };

    bool initGraphicBackend();
        void initDRMLeaseGlobals();
        bool WL_bind_wayland_display { false };
        PFNEGLBINDWAYLANDDISPLAYWL eglBindWaylandDisplayWL { NULL };
        PFNEGLQUERYWAYLANDBUFFERWL eglQueryWaylandBufferWL { NULL };
        PFNGLEGLIMAGETARGETRENDERBUFFERSTORAGEOESPROC glEGLImageTargetRenderbufferStorageOES { NULL };
        PFNGLEGLIMAGETARGETTEXTURE2DOESPROC glEGLImageTargetTexture2DOES { NULL };
        EGLDisplay mainEGLDisplay { EGL_NO_DISPLAY };
        EGLContext mainEGLContext { EGL_NO_CONTEXT };
        LGraphicBackendInterface *graphicBackend { nullptr };
        void *graphicBackendHandle { nullptr };
        void *graphicBackendData { nullptr };
        LCursor *cursor { nullptr };
        CZWeak<LPainter> painter;
        LOutput *currentOutput { nullptr };
        void unitDRMLeaseGlobals();
    void unitGraphicBackend(bool closeLib);

    bool initSeat();
        LSeat *seat { nullptr };
    void unitSeat();

    bool initInputBackend();
        LInputDevice fakeDevice { 7 };
        void *inputBackendHandle { nullptr };
        void *inputBackendData { nullptr };
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

    enum InsertOptions : UInt8
    {
        UpdateSurfaces  = static_cast<UInt8>(1) << 0,
        UpdateLayers    = static_cast<UInt8>(1) << 1
    };

    void notifyOrderChangeFromSurface(LSurface *from);
    void insertSurfaceAfter(LSurface *prevSurface, LSurface *surfaceToInsert, CZBitset<InsertOptions> options);
    void insertSurfaceBefore(LSurface *nextSurface, LSurface *surfaceToInsert, CZBitset<InsertOptions> options);

    std::list<LSurface*>surfaces;
    std::vector<LClient*>clients;
    std::vector<LOutput*>outputs;
    std::vector<LTexture*>textures;
    std::vector<LAnimation*>animations;
    std::vector<LTimer*>oneShotTimers;

    bool runningAnimations();
    void processAnimations();

    // Thread specific data
    struct ThreadData
    {
        ThreadData(LOutput *output) noexcept;
        ~ThreadData();
        CZWeak<LOutput> output;
        LPainter painter;
        std::vector<LRenderBuffer::ThreadData> renderBuffersToDestroy;
        std::unordered_set<int> posixSignalsToDisable;
    };
    std::unordered_map<std::thread::id, ThreadData> threadsMap;
    ThreadData &initThreadData(LOutput *output = nullptr) noexcept;
    void unitThreadData() noexcept;
    void destroyPendingRenderBuffers(std::thread::id *id);
    void addRenderBufferToDestroy(std::thread::id thread, LRenderBuffer::ThreadData &data);
    static LPainter *findPainter();

    // Posix signals
    std::unordered_set<int> posixSignals;
    std::unordered_map<int, wl_event_source*> posixSignalSources;
    bool posixSignalsChanged { false };
    void disablePendingPosixSignals() noexcept; // Called from rendering threads
    void handlePosixSignalChanges() noexcept; // Called from the main thread
    void unitPosixSignals() noexcept;

    void sendPendingConfigurations();
    void sendPresentationTime();
    bool isInputBackendInitialized { false };
    UInt8 screenshotManagers { 0 };

    struct DMAFeedback
    {
        Int32 tableFd { -1 };
        UInt32 tableSize;
        dev_t device;
        wl_array formatIndices;
        wl_array scanoutIndices;
    } dmaFeedback;

    void initDMAFeedback() noexcept;
    void unitDMAFeedback() noexcept;

    std::list<LSurface*> layers[5];
    UInt32 surfaceRaiseAllowedCounter { 0 };

    void handleDestroyedClients();
    std::set<LClient*> destroyedClients;

#if LOUVRE_ASSERT_CHECKS == 1
    void assertSurfacesOrder();
#endif

};

#endif // LCOMPOSITORPRIVATE_H
