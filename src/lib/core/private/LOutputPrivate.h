#ifndef LOUTPUTPRIVATE_H
#define LOUTPUTPRIVATE_H

#include <LOutputFramebuffer.h>
#include <LRenderBuffer.h>
#include <LOutput.h>
#include <LBitset.h>
#include <LGammaTable.h>
#include <atomic>
#include <mutex>
#include <functional>

using namespace Louvre;

struct LOutput::Params
{
    std::function<void(LOutput*)> callback { nullptr };
    void *backendData { nullptr };
};

LPRIVATE_CLASS_NO_COPY(LOutput)

    // TODO: Add timestamp to SRM pageflip events and option for toggling vsync
    std::mutex pageflipMutex {};

    // TODO: Replace with vector
    wl_global *global = nullptr;

    struct PresentationTime
    {
        timespec time;
        UInt32 period;
        UInt64 frame;
        UInt32 flags;
    } presentationTime;

    enum StateFlags : UInt32
    {
        UsingFractionalScale                = 1 << 0,
        FractionalOversamplingEnabled       = 1 << 1,
        PendingRepaint                      = 1 << 2,
        HasUnhandledPresentationTime        = 1 << 3,
        HasDamage                           = 1 << 4
    };

    LOutputPrivate(LOutput *output);

    // Created before initializeGL() and destroyed after uninitializeGL()
    LPainter *painter { nullptr };

    // Using bitset instead of a booleans to save some bytes
    LBitset<StateFlags> stateFlags { FractionalOversamplingEnabled };

    // The current state of the output
    State state { Uninitialized };

    // Handle to the output (some internals require the public API)
    LOutput *output;

    // Wrapper for the graphic backend framebuffer
    LOutputFramebuffer fb;

    // Framebuffer for fractional scaling with oversampling
    LRenderBuffer fractionalFb { LSize(64, 64), false };

    // The wp_fractional_v1 scale set with setScale() returned with fractionalScale()
    Float32 fractionalScale { 1.f };

    // The wl_output scale used for rendering ceil(fractionalScale)
    Float32 scale { 1.f };

    // Transform set with LOutput::setTransform()
    LFramebuffer::Transform transform { LFramebuffer::Normal };

    // Rect in surface coordinates
    LRect rect;

    /* Size in buffer coorinates, width and height are swapped if transform has 90° rotation.
     * It also differs from the current mode size when using a fractional scale */
    LSize sizeB;

    // Checked witin backend...GL() methods below, trigger resizeGL() and moveGL() when != to rect
    LPoint lastPos;
    LSize lastSize;

    /* Damage set with LOutput::setBufferDamage(), used in fractional scaling with oversampling or when
     * the graphic backend uses DUMB buffers or CPU copy. Cleared after page flip. */
    LRegion damage;

    // Thread sync stuff
    std::atomic<bool> callLock;
    std::atomic<bool> callLockACK;
    std::thread::id threadId;

    // Raw native OpenGL textures that need to be destroyed from this thread
    std::vector<GLuint>nativeTexturesToDestroy;

    LGammaTable gammaTable {0};

    // API for the graphic backend
    void *graphicBackendData {nullptr};
    void backendInitializeGL();
    void backendPaintGL();
    void backendResizeGL();
    void backendUninitializeGL();
    void backendPageFlipped();

    bool initialize();
    void updateRect();
    void updateGlobals();
};

#endif // LOUTPUTPRIVATE_H
