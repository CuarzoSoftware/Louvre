#ifndef LSURFACEPRIVATE_H
#define LSURFACEPRIVATE_H

#include <CZ/Louvre/Protocols/Viewporter/viewporter.h>
#include <CZ/Louvre/Protocols/Viewporter/RViewport.h>
#include <CZ/Louvre/Protocols/Wayland/RSurface.h>
#include <CZ/Louvre/Private/LCompositorPrivate.h>
#include <LSurface.h>
#include <CZ/CZBitset.h>
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

    CZBitset<ChangesToNotify> changesToNotify;

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
    };

    CZBitset<StateFlags> stateFlags
    {
        ReceiveInput |
        InfiniteInput |
        BufferReleased |
        VSync
    };

    struct State
    {
        wl_listener onBufferDestroyListener;
        wl_resource *bufferRes              { nullptr };
        bool hasBuffer                      { false };
        Int32 bufferScale                   { 1 };
        CZTransform transform                { CZTransform::Normal };
        LContentType contentType            { LContentTypeNone };
        SkPoint lockedPointerPosHint        { -1.f, -1.f };
    };

    CZWeak<LBaseSurfaceRole> role, prevRole;
    std::unique_ptr<SkRegion> pendingPointerConstraintRegion;
    SkRegion pointerConstraintRegion;

    State current, pending;

    CZWeak<Protocols::PointerConstraints::RLockedPointer> lockedPointerRes;
    CZWeak<Protocols::PointerConstraints::RConfinedPointer> confinedPointerRes;

    SkRect srcRect                          { 0, 0, 1, 1 };
    SkISize size                              { 1, 1 };
    SkISize sizeB                             { 1, 1 };
    SkIPoint pos;
    LTexture *texture                       { nullptr };
    SkRegion currentDamage;
    SkRegion currentTranslucentRegion;
    SkRegion currentOpaqueRegion;
    SkRegion currentInputRegion;
    SkRegion currentInvisibleRegion;

    SkRegion pendingInputRegion;
    SkRegion pendingOpaqueRegion;
    SkRegion pendingTranslucentRegion;
    SkRegion pendingInvisibleRegion;

    std::vector<SkIRect> pendingDamageB;
    std::vector<SkIRect> pendingDamage;
    SkRegion currentDamageB;

    Wayland::RSurface *surfaceResource      { nullptr };

    LTexture *textureBackup;
    LSurface *parent                        { nullptr };
    LSurface *pendingParent                 { nullptr };
    std::list<LSurface*> children;
    std::list<LSurface*> pendingChildren;
    std::list<LSurface*>::iterator parentLink;
    std::list<LSurface*>::iterator pendingParentLink;
    std::vector<Wayland::RCallback*>frameCallbacks;
    UInt32 damageId;
    UInt32 commitId { 0 };
    std::list<LSurface*>::iterator compositorLink;
    std::list<LSurface*>::iterator layerLink;
    LSurfaceLayer layer { LLayerMiddle };
    Int32 lastSentPreferredBufferScale      { -1 };
    CZTransform lastSentPreferredTransform { CZTransform::Normal };
    std::vector<LOutput*> outputs;

    std::vector<PresentationTime::RPresentationFeedback*> presentationFeedbackResources;
    std::vector<Protocols::IdleInhibit::RIdleInhibitor*> idleInhibitorResources;

    CZWeak<LBackgroundBlur> backgroundBlur;
    CZWeak<Protocols::InvisibleRegion::RInvisibleRegion> invisibleRegion;

    // Find the prev surface using layers (returns nullptr if no prev surface)
    LSurface *prevSurfaceInLayers() noexcept;
    void setLayer(LSurfaceLayer layer);
    void sendPresentationFeedback(LOutput *output) noexcept;
    void setPendingParent(LSurface *pendParent) noexcept;
    void setParent(LSurface *parent);
    void removeChild(LSurface *child);
    void setMapped(bool state);
    void setRole(LBaseSurfaceRole *role) noexcept;
    void notifyRoleChange();
    void applyPendingChildren();
    bool bufferToTexture() noexcept;
    void sendPreferredScale() noexcept;
    bool isInChildrenOrPendingChildren(LSurface *child) noexcept;
    bool hasBufferOrPendingBuffer() noexcept;
    void setKeyboardGrabToParent();
    void updateDamage() noexcept;
    bool updateDimensions(Int32 widthB, Int32 heightB) noexcept;
    void simplifyDamage(std::vector<SkIRect> &vec) noexcept;
    void destroyCursorOrDNDRole();
};

#endif // LSURFACEPRIVATE_H
