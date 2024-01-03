#ifndef LSURFACEPRIVATE_H
#define LSURFACEPRIVATE_H

#include <LSurface.h>
#include <private/LCompositorPrivate.h>
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
        InputRegionChanged          = 1 << 5
    };

    UInt32 changesToNotify = NoChanges;

    struct State
    {
        LBaseSurfaceRole *role                          = nullptr;
        wl_resource *buffer                             = nullptr;
        Int32 bufferScale                               = 1;
        LFramebuffer::Transform transform               = LFramebuffer::Normal;
    };

    State current, pending;

    LRectF srcRect = LRectF(0,0,1,1);
    LSize size, sizeB;
    LPoint pos;
    LTexture *texture                                   = nullptr;
    LRegion currentDamage;
    LRegion currentTranslucentRegion;
    LRegion currentOpaqueRegion;
    LRegion currentInputRegion;

    bool destroyed                                      = false;
    bool damaged                                        = false;
    bool minimized                                      = false;
    bool receiveInput                                   = true;
    bool inputRegionIsInfinite                          = true;
    bool bufferReleased                                 = true;
    bool attached                                       = false;
    bool mapped                                         = false;

    LRegion pendingInputRegion;
    LRegion pendingOpaqueRegion;
    LRegion pendingTranslucentRegion;

    std::vector<LRect> pendingDamageB;
    std::vector<LRect> pendingDamage;
    LRegion currentDamageB;

    Wayland::RSurface *surfaceResource = nullptr;
    LSurfaceView *lastPointerEventView = nullptr;

    LTexture *textureBackup;
    LSurface *parent                                    = nullptr;
    LSurface *pendingParent                             = nullptr;
    std::list<LSurfaceView*> views;
    std::list<LSurface*> children;
    std::list<LSurface*> pendingChildren;
    std::list<LSurface*>::iterator parentLink;
    std::list<LSurface*>::iterator pendingParentLink;
    std::list<Wayland::RCallback*>frameCallbacks;
    UInt32 damageId;
    std::list<LSurface*>::iterator compositorLink, clientLink;
    Int32 lastSentPreferredBufferScale = -1;
    std::list<LOutput*> outputs;

    std::list<WpPresentationTime::RWpPresentationFeedback*> wpPresentationFeedbackResources;
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

    inline void updateDamage()
    {        
        if (!texture->initialized() || changesToNotify & (BufferSizeChanged | BufferTransformChanged | BufferScaleChanged))
        {
            currentDamageB.clear();
            currentDamageB.addRect(LRect(0, sizeB));
            currentDamage.clear();
            currentDamage.addRect(LRect(0, size));
        }
        else if (!pendingDamageB.empty() || !pendingDamage.empty())
        {
            while (!pendingDamage.empty())
            {
                LRect &r = pendingDamage.back();
                currentDamageB.addRect((r.x() - 1 )*current.bufferScale,
                                    (r.y() - 1 )*current.bufferScale,
                                    (r.w() + 2 )*current.bufferScale,
                                    (r.h() + 2 )*current.bufferScale);
                pendingDamage.pop_back();
            }

            while (!pendingDamageB.empty())
            {
                LRect &r = pendingDamageB.back();
                currentDamageB.addRect(
                    r.x() - 1,
                    r.y() - 1,
                    r.w() + 2,
                    r.h() + 2);
                pendingDamageB.pop_back();
            }

            currentDamageB.clip(LRect(0, sizeB));
            LRegion::multiply(&currentDamage, &currentDamageB, 1.f/Float32(current.bufferScale));
        }
    }

    inline void updateDimensions(Int32 widthB, Int32 heightB)
    {
        if (LFramebuffer::is90Transform(current.transform))
        {
            sizeB.setW(heightB);
            sizeB.setH(widthB);
        }
        else
        {
            sizeB.setW(widthB);
            sizeB.setH(heightB);
        }

        srcRect.setX(0.f);
        srcRect.setY(0.f);
        srcRect.setW(Float32(sizeB.w()) / Float32(current.bufferScale));
        srcRect.setH(Float32(sizeB.h()) / Float32(current.bufferScale));

        size.setW(roundf(srcRect.w()));
        size.setH(roundf(srcRect.h()));
    }
};

#endif // LSURFACEPRIVATE_H
