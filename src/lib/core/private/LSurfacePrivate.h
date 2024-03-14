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
#include <string>

using namespace Louvre;
using namespace Louvre::Protocols;

struct LSurface::Params
{
    Wayland::RSurface *surfaceResource;
};

LPRIVATE_CLASS(LSurface)

    // Changes that are notified at the end of a commit after all changes are applied
    enum ChangesToNotify : UInt32
    {
        NoChanges                   = 0,
        BufferSizeChanged           = 1 << 0,
        BufferScaleChanged          = 1 << 1,
        BufferTransformChanged      = 1 << 2,
        DamageRegionChanged         = 1 << 3,
        OpaqueRegionChanged         = 1 << 4,
        InputRegionChanged          = 1 << 5,
        SourceRectChanged           = 1 << 6,
        SizeChanged                 = 1 << 7,
        VSyncChanged                = 1 << 8
    };

    LBitset<ChangesToNotify> changesToNotify;

    enum StateFlags : UInt32
    {
        ViewportIsScaled           = 1 << 0,
        ViewportIsCropped          = 1 << 1,
        Destroyed                  = 1 << 2,
        Damaged                    = 1 << 3,
        Minimized                  = 1 << 4,
        ReceiveInput               = 1 << 5,
        InfiniteInput              = 1 << 6,
        BufferReleased             = 1 << 7,
        BufferAttached             = 1 << 8,
        Mapped                     = 1 << 9,
        VSync                      = 1 << 10
    };

    LBitset<StateFlags> stateFlags { ReceiveInput | InfiniteInput | BufferReleased | VSync };

    struct State
    {
        LBaseSurfaceRole *role              { nullptr };
        wl_resource *buffer                 { nullptr };
        Int32 bufferScale                   { 1 };
        LFramebuffer::Transform transform   { LFramebuffer::Normal };
    };

    State current, pending;

    LRectF srcRect                          { 0, 0, 1, 1 };
    LSize size                              { 1, 1 };
    LSize sizeB                             { 1, 1 };
    LPoint pos;
    LTexture *texture                       { nullptr };
    LRegion currentDamage;
    LRegion currentTranslucentRegion;
    LRegion currentOpaqueRegion;
    LRegion currentInputRegion;

    LRegion pendingInputRegion;
    LRegion pendingOpaqueRegion;
    LRegion pendingTranslucentRegion;

    std::vector<LRect> pendingDamageB;
    std::vector<LRect> pendingDamage;
    LRegion currentDamageB;

    Wayland::RSurface *surfaceResource      { nullptr };
    LWeak<LSurfaceView> lastPointerEventView;
    LWeak<LSurfaceView> lastTouchEventView;

    LTexture *textureBackup;
    LSurface *parent                        { nullptr };
    LSurface *pendingParent                 { nullptr };
    std::vector<LSurfaceView*> views;
    std::list<LSurface*> children;
    std::list<LSurface*> pendingChildren;
    std::list<LSurface*>::iterator parentLink;
    std::list<LSurface*>::iterator pendingParentLink;
    std::vector<Wayland::RCallback*>frameCallbacks;
    UInt32 damageId;
    UInt32 commitId { 0 };
    std::list<LSurface*>::iterator compositorLink;
    Int32 lastSentPreferredBufferScale      { -1 };
    LFramebuffer::Transform lastSentPreferredTransform { LFramebuffer::Normal };
    std::vector<LOutput*> outputs;

    std::vector<PresentationTime::RPresentationFeedback*> presentationFeedbackResources;
    void sendPresentationFeedback(LOutput *output);
    void setBufferScale(Int32 scale);
    void setPendingParent(LSurface *pendParent);
    void setParent(LSurface *parent);
    void removeChild(LSurface *child);
    void setMapped(bool state);
    void setPendingRole(LBaseSurfaceRole *role);
    void applyPendingRole();
    void applyPendingChildren();
    bool bufferToTexture();
    void notifyPosUpdateToChildren(LSurface *surface);
    void sendPreferredScale();
    bool isInChildrenOrPendingChildren(LSurface *child);
    bool hasRoleOrPendingRole();
    bool hasBufferOrPendingBuffer();
    void setKeyboardGrabToParent();
    void updateDamage();
    bool updateDimensions(Int32 widthB, Int32 heightB);
};

#endif // LSURFACEPRIVATE_H
