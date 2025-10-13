#ifndef LOUTPUTPRIVATE_H
#define LOUTPUTPRIVATE_H

#include <CZ/Louvre/Seat/LOutput.h>
#include <CZ/Louvre/Roles/LSurface.h>
#include <CZ/Louvre/LMargins.h>
#include <CZ/Ream/RSurface.h>
#include <CZ/Ream/RImage.h>
#include <CZ/Core/Events/CZPresentationEvent.h>
#include <CZ/Core/CZBitset.h>
#include <CZ/Core/CZWeak.h>
#include <future>
#include <list>
#include <queue>

using namespace CZ;

struct LOutput::Params
{
    std::shared_ptr<LBackendOutput> backend;
};

LPRIVATE_CLASS_NO_COPY(LOutput)

    CZWeak<LGlobal> global;

    enum StateFlags : UInt32
    {
        UsingFractionalScale                = static_cast<UInt32>(1) << 0,
        OversamplingEnabled                 = static_cast<UInt32>(1) << 1,
        PendingRepaint                      = static_cast<UInt32>(1) << 2,
        CursorPlaneEnabled                  = static_cast<UInt32>(1) << 3,
        NeedsFullRepaint                    = static_cast<UInt32>(1) << 4,
        IsBlittingFramebuffers              = static_cast<UInt32>(1) << 5,
        IsInPaintGL                         = static_cast<UInt32>(1) << 6,
    };

    LOutputPrivate(LOutput *output) noexcept : output(output) {}

    // Using bitset instead of a booleans to save some bytes
    CZBitset<StateFlags> stateFlags { OversamplingEnabled | CursorPlaneEnabled };

    // The current state of the output
    State state { Uninitialized };

    // Handle to the output (some internals require the public API)
    LOutput *output;

    // Framebuffer for fractional scaling with oversampling
    std::shared_ptr<RSurface> osSurface;

    // The wp_fractional_v1 scale set with setScale() returned with fractionalScale()
    Float32 fractionalScale { 1.f };

    // The wl_output scale used for rendering ceil(fractionalScale)
    Float32 scale { 1.f };

    // Transform set with LOutput::setTransform()
    CZTransform transform { CZTransform::Normal };

    // Rect in surface coordinates
    SkIRect rect { 0, 0, 0, 0 };

    /* Size in buffer coorinates, width and height are swapped if transform has 90° rotation.
     * It also differs from the current mode size when using a fractional scale */
    SkISize bufferSize { 0, 0 };

    // Checked witin backend...GL() methods below, trigger resizeGL() and moveGL() when != to rect
    SkIPoint lastPos { 0, 0 };
    SkISize lastSize { 0, 0 };

    void resizeOSSurface() noexcept;
    void damageToBufferCoords() noexcept;
    void blitFramebuffers() noexcept;
    void blitFractionalScaleFb(bool cursorOnly) noexcept;

    // DRM Lease
    bool leasable { false };
    CZWeak<Protocols::DRMLease::RDRMLease> lease;
    std::vector<Protocols::DRMLease::RDRMLeaseConnector*> drmLeaseConnectorRes;

    // Wlr Output Management
    std::vector<Protocols::WlrOutputManagement::RWlrOutputHead*> wlrOutputHeads;

    // From the last gamma LUT set by a client
    CZWeak<Protocols::GammaControl::RZwlrGammaControlV1> wlrGammaControl;

    // Thread sync stuff
    std::promise<bool> initPromise;
    std::promise<bool> unitPromise;
    std::thread::id threadId;

    CZWeak<LSessionLockRole> sessionLockRole;
    void removeFromSessionLockPendingRepaint() noexcept;

    /* Functions called by the backend, from a render thread */

    // Notifies the successful output initialization
    void backendInitializeGL();
    void backendPaintGL();

    // Notifies that the backend changed the scale or current mode size
    void backendResizeGL() noexcept;
    void backendUninitializeGL();
    void backendPresented(const CZPresentationTime &info) noexcept;
    void backendDiscarded(UInt64 paintEventId) noexcept;
    void updateRect();
    void updateGlobals();

    // Send discard events to unpresented surfaces after a paintGL event
    void handleUnpresentedSurfaces() noexcept;
    // Marked as presented in paintGL but waiting for a page flip/discard
    // Handled when the output sends an CZPresentationEvent
    std::list<CZWeak<Protocols::PresentationTime::RPresentationFeedback>> waitingPresentationFeedback;
    // Presented/discarded frames
    std::queue<CZPresentationEvent> presentationEventQueue;

    std::list<LExclusiveZone*> exclusiveZones;
    SkIRect availableGeometry { 0, 0, 0, 0 };
    LMargins exclusiveEdges;
    void updateExclusiveZones() noexcept;
    void updateLayerSurfacesMapping() noexcept;
};

#endif // LOUTPUTPRIVATE_H
