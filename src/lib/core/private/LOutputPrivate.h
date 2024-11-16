#ifndef LOUTPUTPRIVATE_H
#define LOUTPUTPRIVATE_H

#include <LOutputFramebuffer.h>
#include <LRenderBuffer.h>
#include <LOutput.h>
#include <LBitset.h>
#include <LSurface.h>
#include <LGammaTable.h>
#include <LMargins.h>
#include <atomic>
#include <list>
#include <mutex>
#include <functional>

using namespace Louvre;

struct LOutput::Params
{
    std::function<void(LOutput*)> callback { nullptr };
    void *backendData { nullptr };
};

LPRIVATE_CLASS_NO_COPY(LOutput)

    LWeak<LGlobal> global;

    struct PresentationTime
    {
        timespec time;
        UInt32 period;
        UInt64 frame;
        UInt32 flags;
    } presentationTime;

    std::mutex pageflipMutex;

    enum StateFlags : UInt32
    {
        UsingFractionalScale                = static_cast<UInt32>(1) << 0,
        FractionalOversamplingEnabled       = static_cast<UInt32>(1) << 1,
        PendingRepaint                      = static_cast<UInt32>(1) << 2,
        HasUnhandledPresentationTime        = static_cast<UInt32>(1) << 3,
        CursorEnabled                       = static_cast<UInt32>(1) << 4,
        HwCursorEnabled                     = static_cast<UInt32>(1) << 5,
        CursorRenderedInPrevFrame           = static_cast<UInt32>(1) << 6,
        CursorNeedsRendering                = static_cast<UInt32>(1) << 7,
        ScreenshotsWithCursor               = static_cast<UInt32>(1) << 8,
        ScreenshotsWithoutCursor            = static_cast<UInt32>(1) << 9,
        NeedsFullRepaint                    = static_cast<UInt32>(1) << 10,
        IsBlittingFramebuffers              = static_cast<UInt32>(1) << 11,
        IsInPaintGL                         = static_cast<UInt32>(1) << 12,
        HasScanoutBuffer                    = static_cast<UInt32>(1) << 13,
    };

    LOutputPrivate(LOutput *output);

    // Created before initializeGL() and destroyed after uninitializeGL()
    LPainter *painter { nullptr };

    // Using bitset instead of a booleans to save some bytes
    LBitset<StateFlags> stateFlags { FractionalOversamplingEnabled | HwCursorEnabled | CursorEnabled };

    // The current state of the output
    State state { Uninitialized };

    // Handle to the output (some internals require the public API)
    LOutput *output;

    // Wrapper for the graphic backend framebuffer
    LOutputFramebuffer fb;

    // Framebuffer for fractional scaling with oversampling
    LRenderBuffer fractionalFb { LSize(64, 64) };

    // The wp_fractional_v1 scale set with setScale() returned with fractionalScale()
    Float32 fractionalScale { 1.f };

    // The wl_output scale used for rendering ceil(fractionalScale)
    Float32 scale { 1.f };

    // Transform set with LOutput::setTransform()
    LTransform transform { LTransform::Normal };

    // Rect in surface coordinates
    LRect rect;

    /* Size in buffer coorinates, width and height are swapped if transform has 90° rotation.
     * It also differs from the current mode size when using a fractional scale */
    LSize sizeB;

    // Checked witin backend...GL() methods below, trigger resizeGL() and moveGL() when != to rect
    LPoint lastPos;
    LSize lastSize;

    /* Damage set with LOutput::setBufferDamage(), used in fractional scaling with oversampling or when
     * the graphic backend uses DUMB buffers or CPU copy. */
    UInt64 frame { 0 };
    LRegion damage;
    void damageToBufferCoords() noexcept;
    void blitFramebuffers() noexcept;
    void blitFractionalScaleFb(bool cursorOnly) noexcept;

    // DRM Lease
    bool leasable { false };
    LWeak<Protocols::DRMLease::RDRMLease> lease;
    std::vector<Protocols::DRMLease::RDRMLeaseConnector*> drmLeaseConnectorRes;

    // Wlr Output Management
    std::vector<Protocols::WlrOutputManagement::RWlrOutputHead*> wlrOutputHeads;

    // Thread sync stuff
    std::atomic<bool> callLock;
    std::atomic<bool> callLockACK;
    std::thread::id threadId;
    LGammaTable gammaTable {0};

    UInt32 dirtyCursorFBs;
    UInt32 prevCursorSerial;
    LRect prevCursorRect; // Local
    LRegion cursorDamage;
    void calculateCursorDamage() noexcept;
    void drawCursor() noexcept;

    LWeak<LSessionLockRole> sessionLockRole;
    void removeFromSessionLockPendingRepaint() noexcept;

    std::vector<LScreenshotRequest*> screenshotRequests;
    void validateScreenshotRequests() noexcept;
    void handleScreenshotRequests(bool withCursor) noexcept;
    UInt8 screenshotCursorTimeout { 0 };

    struct ScanoutBuffer
    {
        wl_listener bufferDestroyListener
        {
            .link {0},
            .notify = [](wl_listener *listener, void *)
            {
                ScanoutBuffer *scanoutBuffer = (ScanoutBuffer*)listener;
                scanoutBuffer->buffer = nullptr;
                scanoutBuffer->surface.reset();
            }
        };
        wl_buffer *buffer { nullptr };
        LWeak<LSurface> surface;
    } scanout[2]; // Pending and current
    LWeak<LTexture> customScanoutBuffer;

    bool isBufferScannedByOtherOutputs(wl_buffer *buffer) const noexcept;
    void releaseScanoutBuffer(UInt8 index) noexcept;

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

    std::list<LExclusiveZone*> exclusiveZones;
    LRect availableGeometry;
    LMargins exclusiveEdges;
    void updateExclusiveZones() noexcept;
    void updateLayerSurfacesMapping() noexcept;

#if LOUVRE_USE_SKIA == 1
    sk_sp<SkSurface> skSurface;
    void updateSkSurface() noexcept;
#endif
};

#endif // LOUTPUTPRIVATE_H
