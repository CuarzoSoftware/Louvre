#ifndef LCOMPOSITORPRIVATE_H
#define LCOMPOSITORPRIVATE_H

#include <LCompositor.h>

class Louvre::LCompositor::LCompositorPrivate
{
public:
    LCompositorPrivate()                                        = default;
    ~LCompositorPrivate()                                       = default;

    LCompositorPrivate(const LCompositorPrivate&)               = delete;
    LCompositorPrivate &operator=(const LCompositorPrivate&)    = delete;

    struct RemovedOutputGlobal
    {
        wl_global *global;
        UInt32 loopIterations                                   = 0;
    };

    LCompositor *compositor                                     = nullptr;
    LCursor *cursor                                             = nullptr;
    LSeat *seat                                                 = nullptr;
    LSession *session                                           = nullptr;
    Int32 globalScale                                           = 1;
    bool graphicBackendInitialized                              = false;
    bool inputBackendInitialized                                = false;


    std::thread::id threadId;
    mutex renderMutex;

    bool loadGraphicBackend(const char *path);
    bool loadInputBackend(const char *path);

    void raiseChildren(LSurface *surface);
    void insertSurfaceAfter(LSurface *prevSurface,LSurface *surfaceToInsert);
    void insertSurfaceBefore(LSurface *nextSurface,LSurface *surfaceToInsert);
    void updateGlobalScale();

    // Clients
    list<LClient*>clients;

    // Outputs
    list<LOutput*>outputs;

    // Surfaces
    list<LSurface*>surfaces;

    // Output Manager
    LOutputManager *outputManager                               = nullptr;

    // Removed Outputs
    list<RemovedOutputGlobal*>removedOutputGobals;

    int waylandFd;

    LPainter *painter;

    bool started                                                = false;
    LGraphicBackendInterface *graphicBackend                    = nullptr;
    LInputBackendInterface *inputBackend                        = nullptr;

    // Dylib
    void *graphicBackendHandle                                  = nullptr;
    void *inputBackendHandle                                    = nullptr;
    void *graphicBackendData                                    = nullptr;



};


#endif // LCOMPOSITORPRIVATE_H
