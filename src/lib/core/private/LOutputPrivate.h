#ifndef LOUTPUTPRIVATE_H
#define LOUTPUTPRIVATE_H

#include <LOutput.h>
#include <private/LRenderBufferPrivate.h>
#include <atomic>

LPRIVATE_CLASS(LOutput)
    LOutputFramebuffer *fb;
    LFramebuffer::Transform transform = LFramebuffer::Normal;
    LOutput *output;
    LRect rect;
    LPoint lastPos;
    LSize lastSize;
    LSize sizeB;
    std::atomic<bool> callLock;
    std::atomic<bool> callLockACK;
    std::thread::id threadId;

    // Painter
    LPainter *painter = nullptr;

    State state = Uninitialized;

    void *graphicBackendData = nullptr;
    bool pendingRepaint = false;

    // Setup Methods
    bool initialize();

    // Compositor
    wl_global *global = nullptr;

    // Params
    Int32 outputScale = 1;

    timespec presentationTime;

    // Called by the backend
    void backendInitializeGL();
    void backendPaintGL();
    void backendResizeGL();
    void backendUninitializeGL();
    void backendPageFlipped();

    void updateRect();
    void updateGlobals();
    std::list<GLuint>nativeTexturesToDestroy;
};

#endif // LOUTPUTPRIVATE_H
