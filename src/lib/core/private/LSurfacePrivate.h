#ifndef LSURFACEPRIVATE_H
#define LSURFACEPRIVATE_H

#include <protocols/Viewporter/viewporter.h>
#include <protocols/Viewporter/RViewport.h>
#include <protocols/Wayland/RSurface.h>
#include <private/LCompositorPrivate.h>
#include <LSurfaceView.h>
#include <LSurface.h>
#include <LBitset.h>
#include <vector>

using namespace Louvre;
using namespace Louvre::Protocols;

struct LSurface::Params
{
    Wayland::RSurface *surfaceResource;
};

LPRIVATE_CLASS(LSurface)

    // Changes that are notified at the end of a commit after all changes are applied
    enum ChangesToNotify : UInt16
    {
        NoChanges                       = static_cast<UInt16>(0),
        BufferSizeChanged               = static_cast<UInt16>(1) << 0,
        BufferScaleChanged              = static_cast<UInt16>(1) << 1,
        BufferTransformChanged          = static_cast<UInt16>(1) << 2,
        DamageRegionChanged             = static_cast<UInt16>(1) << 3,
        OpaqueRegionChanged             = static_cast<UInt16>(1) << 4,
        InputRegionChanged              = static_cast<UInt16>(1) << 5,
        SourceRectChanged               = static_cast<UInt16>(1) << 6,
        SizeChanged                     = static_cast<UInt16>(1) << 7,
        VSyncChanged                    = static_cast<UInt16>(1) << 8,
        PointerConstraintRegionChanged  = static_cast<UInt16>(1) << 9,
        LockedPointerPosHintChanged     = static_cast<UInt16>(1) << 10,
        ContentTypeChanged              = static_cast<UInt16>(1) << 11,
        InvisibleRegionChanged          = static_cast<UInt16>(1) << 12,
    };

    LBitset<ChangesToNotify> changesToNotify;

    enum StateFlags : UInt16
    {
        ViewportIsScaled            = static_cast<UInt16>(1) << 0,
        ViewportIsCropped           = static_cast<UInt16>(1) << 1,
        Destroyed                   = static_cast<UInt16>(1) << 2,
        Damaged                     = static_cast<UInt16>(1) << 3,
        Minimized                   = static_cast<UInt16>(1) << 4,
        ReceiveInput                = static_cast<UInt16>(1) << 5,
        InfiniteInput               = static_cast<UInt16>(1) << 6,
        BufferReleased              = static_cast<UInt16>(1) << 7,
        BufferAttached              = static_cast<UInt16>(1) << 8,
        Mapped                      = static_cast<UInt16>(1) << 9,
        VSync                       = static_cast<UInt16>(1) << 10,
        ChildrenListChanged         = static_cast<UInt16>(1) << 11,
        ParentCommitNotified        = static_cast<UInt16>(1) << 12,
        InfiniteInvisible           = static_cast<UInt16>(1) << 13,
        UnnotifiedRoleChange        = static_cast<UInt16>(1) << 14,
        AboveParent                 = static_cast<UInt16>(1) << 15,
    };

    LBitset<StateFlags> stateFlags
    {
        ReceiveInput |
        InfiniteInput |
        BufferReleased |
        VSync |
        AboveParent
    };

    struct State
    {
        wl_listener onBufferDestroyListener;
        wl_resource *bufferRes              { nullptr };
        bool hasBuffer                      { false };
        Int32 bufferScale                   { 1 };
        LTransform transform                { LTransform::Normal };
        LContentType contentType            { LContentTypeNone };
        LPointF lockedPointerPosHint        { -1.f, -1.f };
    };

    LWeak<LBaseSurfaceRole> role, prevRole;
    LWeak<Protocols::XdgShell::RXdgSurface> xdgSurface;
    std::unique_ptr<LRegion> pendingPointerConstraintRegion;
    LRegion pointerConstraintRegion;

    State current, pending;

    LWeak<Protocols::PointerConstraints::RLockedPointer> lockedPointerRes;
    LWeak<Protocols::PointerConstraints::RConfinedPointer> confinedPointerRes;

    LRectF srcRect                          { 0, 0, 1, 1 };
    LSize size                              { 1, 1 };
    LSize sizeB                             { 1, 1 };
    LPoint pos;
    LTexture *texture                       { nullptr };
    LRegion currentDamage;
    LRegion currentTranslucentRegion;
    LRegion currentOpaqueRegion;
    LRegion currentInputRegion;
    LRegion currentInvisibleRegion;

    LRegion pendingInputRegion;
    LRegion pendingOpaqueRegion;
    LRegion pendingTranslucentRegion;
    LRegion pendingInvisibleRegion;

    std::vector<LRect> pendingDamageB;
    std::vector<LRect> pendingDamage;
    LRegion currentDamageB;

    Wayland::RSurface *surfaceResource      { nullptr };
    LWeak<LSurfaceView> lastPointerEventView;
    LWeak<LSurfaceView> lastTouchEventView;

    LTexture *textureBackup;
    LSurface *parent                        { nullptr };
    std::vector<LSurfaceView*> views;
    std::list<LSurface*> children;
    std::list<LSurface*>::iterator parentLink;
    std::vector<Wayland::RCallback*>frameCallbacks;
    UInt32 damageId;
    UInt32 commitId { 0 };
    std::list<LSurface*>::iterator compositorLink;
    std::list<LSurface*>::iterator layerLink;
    LSurfaceLayer layer { LLayerMiddle };
    Int32 lastSentPreferredBufferScale      { -1 };
    LTransform lastSentPreferredTransform { LTransform::Normal };
    std::vector<LOutput*> outputs;

    std::vector<PresentationTime::RPresentationFeedback*> presentationFeedbackResources;
    std::vector<Protocols::IdleInhibit::RIdleInhibitor*> idleInhibitorResources;

    LWeak<LBackgroundBlur> backgroundBlur;
    LWeak<Protocols::InvisibleRegion::RInvisibleRegion> invisibleRegion;

    // Find the prev surface using layers (returns nullptr if no prev surface)
    LSurface *prevSurfaceInLayers() noexcept;
    void setLayer(LSurfaceLayer layer);
    void sendPresentationFeedback(LOutput *output) noexcept;
    void setParent(LSurface *parent);
    void removeChild(LSurface *child);
    void setMapped(bool state);
    void setRole(LBaseSurfaceRole *role) noexcept;
    void notifyRoleChange();
    bool bufferToTexture() noexcept;
    void sendPreferredScale() noexcept;
    bool isInChildren(LSurface *child) noexcept;
    bool hasBufferOrPendingBuffer() noexcept;
    void setKeyboardGrabToParent();
    void updateDamage() noexcept;
    bool updateDimensions(Int32 widthB, Int32 heightB) noexcept;
    void simplifyDamage(std::vector<LRect> &vec) noexcept;
    void destroyCursorOrDNDRole();
    bool canHostRole() const noexcept;
};

#endif // LSURFACEPRIVATE_H
