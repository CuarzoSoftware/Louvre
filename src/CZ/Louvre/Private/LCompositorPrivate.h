#ifndef LCOMPOSITORPRIVATE_H
#define LCOMPOSITORPRIVATE_H

#include <CZ/Louvre/LCompositor.h>
#include <CZ/Louvre/Seat/LOutput.h>
#include <CZ/Louvre/Roles/LSurface.h>

#include <CZ/Core/CZEventSource.h>
#include <CZ/Core/CZWeak.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <sys/epoll.h>
#include <unordered_map>
#include <unistd.h>
#include <filesystem>
#include <set>
#include <unordered_set>

using namespace CZ;

LPRIVATE_CLASS(LCompositor)

    std::shared_ptr<RCore> ream;
    std::shared_ptr<CZCore> core;
    int czFd { -1 };
    CompositorState state { CompositorState::Uninitialized };
    std::vector<LGlobal*> globals;
    void processRemovedGlobals();
    void unitCompositor();

    bool initWayland();
        wl_display *display { nullptr };
        std::shared_ptr<CZEventSource> waylandEventSource;
        wl_listener clientConnectedListener;
        LSessionLockManager *sessionLockManager { nullptr };
        LActivationTokenManager *activationTokenManager { nullptr };
    void unitWayland();

    bool initBackend() noexcept;
        std::shared_ptr<LBackend> backend;
        void initDRMLeaseGlobals();
        void unitDRMLeaseGlobals();
        std::unique_ptr<LCursor> cursor;
        LOutput *currentOutput { nullptr }; // The current output handling a paintGL event
        bool initSeat();
        LSeat *seat { nullptr };
        void unitSeat();
    void unitBackend();

    // Event FD to force unlock main loop poll
    void unlockPoll();
    std::thread::id threadId;

    bool loadGraphicBackend(const std::filesystem::path &path);
    bool loadInputBackend(const std::filesystem::path &path);

    std::list<LSurface*> surfaces;
    std::vector<LClient*> clients;
    std::vector<LOutput*> outputs;
    std::array<std::list<LSurface*>, 5> layers;

    // Thread specific data
    struct ThreadData
    {
        ThreadData(LOutput *output) noexcept;
        ~ThreadData();
        CZWeak<LOutput> output;
        std::unordered_set<int> posixSignalsToDisable;
    };
    std::unordered_map<std::thread::id, ThreadData> threadsMap;
    ThreadData &initThreadData(LOutput *output = nullptr) noexcept;
    void unitThreadData() noexcept;

    std::mutex presentationMutex;
    void dispatchPresentationTimeEvents() noexcept;

    // Posix signals
    std::unordered_set<int> posixSignals;
    std::unordered_map<int, wl_event_source*> posixSignalSources;
    bool posixSignalsChanged { false };
    void disablePendingPosixSignals() noexcept; // Called from rendering threads
    void handlePosixSignalChanges() noexcept; // Called from the main thread
    void unitPosixSignals() noexcept;

    void sendPendingConfigurations();

    void handleDestroyedClients();
    std::set<LClient*> destroyedClients;

    void handleUnreleasedBuffers() noexcept;
    std::list<LSurfaceBuffer> unreleasedBuffers;
};

#endif // LCOMPOSITORPRIVATE_H
