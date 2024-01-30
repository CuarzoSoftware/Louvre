#ifndef LSURFACEPRIVATE_H
#define LSURFACEPRIVATE_H

#include <protocols/Viewporter/viewporter.h>
#include <protocols/Viewporter/RViewport.h>
#include <LSurface.h>
#include <private/LCompositorPrivate.h>
#include <vector>
#include <string>
#include <LBitfield.h>

using namespace Louvre;
using namespace Louvre::Protocols;

struct LSurface::Params
{
    Wayland::RSurface *surfaceResource;
};

LPRIVATE_CLASS(LSurface)

    // Changes that are notified at the end of a commit after all changes are applied
    enum ChangesToNotify
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
    };

    LBitfield<ChangesToNotify> changesToNotify;

    enum StateFlags : UInt32
    {
        ViewportIsScaled           = 1 << 0,
        ViewportIsCropped          = 1 << 1,
    };

    LBitfield<StateFlags> stateFlags;

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
    std::vector<LSurfaceView*> views;
    std::list<LSurface*> children;
    std::list<LSurface*> pendingChildren;
    std::list<LSurface*>::iterator parentLink;
    std::list<LSurface*>::iterator pendingParentLink;
    std::vector<Wayland::RCallback*>frameCallbacks;
    UInt32 damageId;
    std::list<LSurface*>::iterator compositorLink;
    Int32 lastSentPreferredBufferScale = -1;
    LFramebuffer::Transform lastSentPreferredTransform = LFramebuffer::Normal;
    std::vector<LOutput*> outputs;

    std::vector<WpPresentationTime::RWpPresentationFeedback*> wpPresentationFeedbackResources;
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
        if (!texture->initialized() || changesToNotify.check(SizeChanged | SourceRectChanged | BufferSizeChanged | BufferTransformChanged | BufferScaleChanged))
        {
            currentDamageB.clear();
            currentDamageB.addRect(LRect(0, sizeB));
            currentDamage.clear();
            currentDamage.addRect(LRect(0, size));
        }
        else if (!pendingDamageB.empty() || !pendingDamage.empty())
        {
            if (stateFlags.check(ViewportIsScaled | ViewportIsCropped))
            {
                Float32 xInvScale = (Float32(current.bufferScale) * srcRect.w())/Float32(size.w());
                Float32 yInvScale = (Float32(current.bufferScale) * srcRect.h())/Float32(size.h());

                Int32 xOffset = roundf(srcRect.x() * Float32(current.bufferScale)) - 2;
                Int32 yOffset = roundf(srcRect.y() * Float32(current.bufferScale)) - 2;

                while (!pendingDamage.empty())
                {
                    LRect &r = pendingDamage.back();
                    currentDamageB.addRect((r.x() * xInvScale + xOffset),
                                        (r.y() * yInvScale + yOffset),
                                        (r.w() * xInvScale + 4 ),
                                        (r.h() * yInvScale + 4 ));
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

                currentDamage = currentDamageB;
                currentDamage.offset(-xOffset - 2, -yOffset - 2);
                currentDamage.multiply(1.f/xInvScale, 1.f/yInvScale);
            }
            else
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
    }

    inline bool updateDimensions(Int32 widthB, Int32 heightB)
    {
        LSize prevSizeB = sizeB;
        LSize prevSize = size;
        LRectF prevSrcRect = srcRect;

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

        if (prevSizeB != sizeB)
            changesToNotify.add(BufferSizeChanged);

        if (surfaceResource->viewportResource())
        {
            bool usingViewportSrc = false;

            // Using the viewport source rect
            if (surfaceResource->viewportResource()->srcRect().x() != -1.f ||
                surfaceResource->viewportResource()->srcRect().y() != -1.f ||
                surfaceResource->viewportResource()->srcRect().w() != -1.f ||
                surfaceResource->viewportResource()->srcRect().h() != -1.f)
            {
                usingViewportSrc = true;

                srcRect = surfaceResource->viewportResource()->srcRect();

                if (srcRect.x() < 0.f || srcRect.y() < 0.f || srcRect.w() <= 0.f || srcRect.h() <= 0.f)
                {
                    wl_resource_post_error(surfaceResource->viewportResource()->resource(),
                                           WP_VIEWPORT_ERROR_BAD_VALUE,
                                           "Invalid source rect (%f, %f, %f, %f).",
                                           srcRect.x(), srcRect.y(), srcRect.w(), srcRect.h());
                    return false;
                }

                if (roundf((srcRect.x() + srcRect.w()) * Float32(current.bufferScale)) > sizeB.w() || roundf((srcRect.y() + srcRect.h()) * Float32(current.bufferScale)) > sizeB.h())
                {
                    wl_resource_post_error(surfaceResource->viewportResource()->resource(),
                                           WP_VIEWPORT_ERROR_OUT_OF_BUFFER,
                                           "Source rectangle extends outside of the content area rect.");
                    return false;
                }

                stateFlags.add(ViewportIsCropped);
            }

            // Using the entire buffer as source
            else
            {
                srcRect.setX(0.f);
                srcRect.setY(0.f);
                srcRect.setW(Float32(sizeB.w()) / Float32(current.bufferScale));
                srcRect.setH(Float32(sizeB.h()) / Float32(current.bufferScale));
                stateFlags.remove(ViewportIsCropped);
            }

            // Using the viewport destination size
            if (surfaceResource->viewportResource()->dstSize().w() != -1 || surfaceResource->viewportResource()->dstSize().h() != -1)
            {
                size = surfaceResource->viewportResource()->dstSize();

                if (size.w() <= 0 || size.h() <= 0)
                {
                    wl_resource_post_error(surfaceResource->viewportResource()->resource(),
                                           WP_VIEWPORT_ERROR_BAD_VALUE,
                                           "Invalid destination size (%d, %d).",
                                           size.w(), size.h());
                    return false;
                }

                stateFlags.add(ViewportIsScaled);
            }

            // Using the viewport source rect size or normal surface size
            else
            {
                if (usingViewportSrc)
                {
                    if (fmod(srcRect.w(), 1.f) != 0.f || fmod(srcRect.h(), 1.f) != 0.f)
                    {
                        wl_resource_post_error(surfaceResource->viewportResource()->resource(),
                                               WP_VIEWPORT_ERROR_BAD_SIZE,
                                               "Destination size is not integer");
                        return false;
                    }

                    size.setW(srcRect.w());
                    size.setH(srcRect.h());
                    stateFlags.add(ViewportIsScaled);
                }
                else
                {
                    size.setW(roundf(srcRect.w()));
                    size.setH(roundf(srcRect.h()));
                    stateFlags.remove(ViewportIsScaled);
                }
            }
        }

        // Normal case, surface has no viewport
        else
        {
            srcRect.setX(0.f);
            srcRect.setY(0.f);
            srcRect.setW(Float32(sizeB.w()) / Float32(current.bufferScale));
            srcRect.setH(Float32(sizeB.h()) / Float32(current.bufferScale));
            size.setW(roundf(srcRect.w()));
            size.setH(roundf(srcRect.h()));
            stateFlags.remove(ViewportIsCropped);
            stateFlags.remove(ViewportIsScaled);
        }

        if (prevSize != size)
            changesToNotify.add(SizeChanged);

        if (prevSrcRect != srcRect)
            changesToNotify.add(SourceRectChanged);

        return true;
    }
};

#endif // LSURFACEPRIVATE_H
