#ifndef LSURFACEPRIVATE_H
#define LSURFACEPRIVATE_H

#include <CZ/Louvre/Protocols/Viewporter/viewporter.h>
#include <CZ/Louvre/Protocols/Viewporter/RViewport.h>
#include <CZ/Louvre/Protocols/Wayland/RWlSurface.h>
#include <CZ/Louvre/Private/LCompositorPrivate.h>
#include <CZ/Louvre/Private/LResourceRef.h>
#include <CZ/Louvre/Events/LSurfaceCommitEvent.h>
#include <CZ/Louvre/Roles/LSurface.h>
#include <CZ/Ream/RImage.h>
#include <CZ/Core/CZBitset.h>
#include <vector>

using namespace CZ;
using namespace CZ::Protocols;

struct LSurface::Params
{
    Wayland::RWlSurface *surfaceResource;
};

LPRIVATE_CLASS(LSurface)

    enum StateFlags : UInt16
    {
        ViewportIsScaled            = static_cast<UInt16>(1) << 0,
        ViewportIsCropped           = static_cast<UInt16>(1) << 1,
        Destroyed                   = static_cast<UInt16>(1) << 2,
        Damaged                     = static_cast<UInt16>(1) << 3,
        SubsurfacesListChanged      = static_cast<UInt16>(1) << 4,
        ReceiveInput                = static_cast<UInt16>(1) << 5,
        InfiniteInput               = static_cast<UInt16>(1) << 6,
        Mapped                      = static_cast<UInt16>(1) << 9,
        VSync                       = static_cast<UInt16>(1) << 10,
        ChildrenListChanged         = static_cast<UInt16>(1) << 11,
        ParentCommitNotified        = static_cast<UInt16>(1) << 12,
        InfiniteInvisible           = static_cast<UInt16>(1) << 13,
        CurrentImageIsSHM           = static_cast<UInt16>(1) << 15,
    };

    CZBitset<StateFlags> stateFlags { ReceiveInput | InfiniteInput | VSync };

    struct Uncommitted
    {
        UInt32 lockCount { 0 };
        CZBitset<LSurfaceCommitEvent::Changes> changesToNotify;
        UInt32 commitId                     { 0 };
        Int32 scale                         { 1 };
        CZTransform transform               { CZTransform::Normal };
        RContentType contentType            { RContentType::Graphics };
        SkRegion inputRegion;
        SkRegion opaqueRegion;
        SkRegion invisibleRegion;
        std::vector<SkIRect> damage, bufferDamage;
        LFrameCallbacks frames;
        std::vector<CZWeak<LSubsurfaceRole>> subsurfacesAbove;
        std::vector<CZWeak<LSubsurfaceRole>> subsurfacesBelow;

        SkPoint lockedPointerPosHint { -1.f, -1.f };
        std::shared_ptr<SkRegion> pointerConstraintRegion;
        CZWeak<PointerConstraints::RLockedPointer> lockedPointerRes;
        CZWeak<PointerConstraints::RConfinedPointer> confinedPointerRes;

        std::list<CZWeak<PresentationTime::RPresentationFeedback>> presentationFeedbackRes;

        CZWeak<DRMSyncObj::RDRMSyncObjSurface> drmSyncObjSurfaceRes;
        LSurfaceBuffer buffer {};
    };

    struct Committed
    {
        UInt32 lockCount { 0 };
        CZBitset<LSurfaceCommitEvent::Changes> changesToNotify;
        UInt32 commitId                     { 0 };
        Int32 scale                         { 1 };
        CZTransform transform               { CZTransform::Normal };
        RContentType contentType            { RContentType::Graphics };
        SkRegion inputRegion;
        SkRegion opaqueRegion;
        SkRegion invisibleRegion;
        SkRegion damage, bufferDamage;
        LFrameCallbacks frames;
        std::vector<LSubsurfaceRole*> subsurfacesAbove;
        std::vector<LSubsurfaceRole*> subsurfacesBelow;

        PointerConstraintMode pointerConstraintMode { PointerConstraintMode::Free };
        SkPoint lockedPointerPosHint        { -1.f, -1.f };
        SkRegion pointerConstraintRegion;
        CZWeak<PointerConstraints::RLockedPointer> lockedPointerRes;
        CZWeak<PointerConstraints::RConfinedPointer> confinedPointerRes;

        std::list<CZWeak<PresentationTime::RPresentationFeedback>> presentationFeedbackRes;

        CZWeak<DRMSyncObj::RDRMSyncObjSurface> drmSyncObjSurfaceRes;
        LSurfaceBuffer buffer {};
        std::shared_ptr<RImage> image;
    };

    SkIPoint pos                              { 0, 0 };
    SkISize size                              { 1, 1 };
    SkRect srcRect                            { 0, 0, 1, 1 };
    SkISize sizeB                             { 1, 1 };

    CZWeak<LBaseSurfaceRole> role;
    Wayland::RWlSurface *surfaceResource      { nullptr };
    CZWeak<Protocols::XdgShell::RXdgSurface> xdgSurface;

    /* Committed and applied */
    Committed current;

    /* Uncommitted */
    Uncommitted pending;

    /* Committed but not yet applied (locked) */
    std::list<std::shared_ptr<Uncommitted>> cached;

    /* Locks created by timelines */
    std::vector<std::shared_ptr<LSurfaceLock>> acquireTimelineLocks;

    UInt32 damageId {};
    Int32 lastSentPreferredBufferScale      { -1 };
    CZTransform lastSentPreferredTransform { CZTransform::Normal };
    std::unordered_set<LOutput*> outputs;

    std::list<LSurface*>::iterator compositorLink;
    std::list<LSurface*>::iterator layerLink;
    LSurfaceLayer layer { LLayerMiddle };
    CZWeak<LSurface> parent;

    std::vector<Protocols::IdleInhibit::RIdleInhibitor*> idleInhibitorResources;
    CZWeak<LBackgroundBlur> backgroundBlur;
    CZWeak<Protocols::InvisibleRegion::RInvisibleRegion> invisibleRegion;

    std::shared_ptr<LSurfaceLock> lock() noexcept;
    void setLayer(LSurfaceLayer layer) noexcept;
    void setParent(LSurface *parent) noexcept;

    // Note: notifyLater can be set to true only if called from applyCommit()
    void setMapped(bool state, bool notifyLater = false) noexcept;
    void setRole(LBaseSurfaceRole *role, bool notify) noexcept;
    void notifyRoleChange() noexcept;
    void sendPreferredScale() noexcept;
    bool hasBufferOrPendingBuffer() noexcept;
    void setKeyboardGrabToParent();
    void destroyCursorOrDNDRole();
    bool canHostRole() const noexcept;

    bool bufferToImage(Uncommitted &pending) noexcept;
    void updateDamage(Uncommitted &pending) noexcept;
    bool updateDimensions(Int32 widthB, Int32 heightB) noexcept;

    void checkTimelines() noexcept;
    void handleCommit() noexcept;
    void unlockCommit(UInt32 commitId) noexcept;
    void applyCommit(Uncommitted &pending) noexcept;
    void clearUncommitted(Uncommitted &pending) noexcept;
    void applySubsurfacesOrder(Uncommitted &pending) noexcept;
    bool notifyCommitToSubsurfaces() noexcept;

    // true if surface (must be a wl_subsurface) is descendant or equal to parent (uses the pending state)
    static bool IsSubsurfaceOf(LSurface *surface, LSurface *parent) noexcept;
};

#endif // LSURFACEPRIVATE_H
