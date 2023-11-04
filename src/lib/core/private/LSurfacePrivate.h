#ifndef LSURFACEPRIVATE_H
#define LSURFACEPRIVATE_H

#include <LSurface.h>
#include <private/LCompositorPrivate.h>
#include <vector>

using namespace Louvre;
using namespace Louvre::Protocols;

struct LSurface::Params
{
    Wayland::RSurface *surfaceResource;
};

LPRIVATE_CLASS(LSurface)
    // Called from LCompositor constructor
    static void getEGLFunctions();
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

    // Input
    void setKeyboardGrabToParent();

    struct State
    {
        LBaseSurfaceRole *role                          = nullptr;
        wl_resource *buffer                             = nullptr;
        Int32 bufferScale                               = 1;
    };

    // Link in LSurfaces compositor list
    std::list<LSurface*>::iterator compositorLink, clientLink;

    LRegion pendingInputRegion;
    LRegion currentInputRegion;

    LRegion pendingOpaqueRegion;
    LRegion currentOpaqueRegion;

    LRegion pendingTranslucentRegion;
    LRegion currentTranslucentRegion;

    LSize currentSizeB;
    LSize currentSize;

    std::vector<LRect> pendingDamageB;
    std::vector<LRect> pendingDamage;
    LRegion currentDamageB;
    LRegion currentDamage;

    LTexture *texture                                   = nullptr;
    LTexture *textureBackup;

    std::list<LSurface*> children;
    std::list<LSurface*> pendingChildren;

    // Buffer
    void setBufferScale(Int32 scale);

    LSurface *parent                                    = nullptr;
    LSurface *pendingParent                             = nullptr;
    std::list<LSurface*>::iterator parentLink;
    std::list<LSurface*>::iterator pendingParentLink;

    Wayland::RSurface *surfaceResource = nullptr;

    std::list<Wayland::RCallback*>frameCallbacks;

    State current, pending;

    std::list<LOutput*> outputs;

    bool destroyed                                      = false;
    bool damaged                                        = false;
    UInt32 damageId;
    LPoint pos;
    bool minimized                                      = false;
    bool receiveInput                                   = true;
    bool opaqueRegionChanged                            = false;
    bool inputRegionChanged                             = false;
    bool inputRegionIsInfinite                          = true;
    bool damagesChanged                                 = false;
    bool bufferSizeChanged                              = false;
    bool bufferReleased                                 = true;
    bool attached                                       = false;
    bool mapped                                         = false;

    // Presentation feedback
    std::list<WpPresentationTime::RWpPresentationFeedback*> wpPresentationFeedbackResources;
    void sendPresentationFeedback(LOutput *output, timespec &ns);

    LSurfaceView *lastPointerEventView = nullptr;

    Int32 lastSentPreferredBufferScale = -1;

    inline void damageCalc1()
    {
        if (compositor()->imp()->greatestOutputScale == 1)
        {
            while (!pendingDamage.empty())
            {
                currentDamageB.addRect(pendingDamage.back());
                pendingDamage.pop_back();
            }
        }
        else
        {
            while (!pendingDamage.empty())
            {
                LRect &r = pendingDamage.back();
                currentDamageB.addRect(r.x() - 1,
                                       r.y() - 1 ,
                                       r.w() + 2,
                                       r.h() + 2);
                pendingDamage.pop_back();
            }
        }
    }

    inline void damageCalc2()
    {
        if (compositor()->imp()->greatestOutputScale == 1)
        {
            while (!pendingDamage.empty())
            {
                LRect &r = pendingDamage.back();
                currentDamageB.addRect(r.x() << 1,
                                       r.y() << 1,
                                       r.w() << 1,
                                       r.h() << 1),
                pendingDamage.pop_back();
            }
        }
        else
        {
            while (!pendingDamage.empty())
            {
                LRect &r = pendingDamage.back();
                currentDamageB.addRect((r.x() - 1 ) << 1,
                                       (r.y() - 1 ) << 1,
                                       (r.w() + 2 ) << 1,
                                       (r.h() + 2 ) << 1),
                pendingDamage.pop_back();
            }
        }
    }

    inline void damageCalcN()
    {
        if (compositor()->imp()->greatestOutputScale == 1)
        {
            while (!pendingDamage.empty())
            {
                currentDamageB.addRect(pendingDamage.back()*current.bufferScale);
                pendingDamage.pop_back();
            }
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
        }
    }

    inline void damageMod()
    {
        if (compositor()->imp()->greatestOutputScale == 1)
        {
            while (!pendingDamageB.empty())
            {
                currentDamageB.addRect(pendingDamageB.back());
                pendingDamageB.pop_back();
            }
        }
        else
        {
            Int32 modX, modY, modW, modH;
            while (!pendingDamageB.empty())
            {
                LRect &r = pendingDamageB.back();
                modX = r.x() % current.bufferScale;
                modY = r.y() % current.bufferScale;
                modW = r.w() % current.bufferScale;
                modH = r.h() % current.bufferScale;
                currentDamageB.addRect(
                    r.x() - modX,
                    r.y() - modY,
                    r.w() + modW + (modX << 1),
                    r.h() + modH + (modY << 1));
                pendingDamageB.pop_back();
            }
        }
    }

    inline void damageNormal(Int32 width, Int32 height, const LSize &prevSize, bool bufferScaleChanged)
    {
        LSize newSize = LSize(width, height);

        if (newSize != prevSize || bufferScaleChanged || !texture->initialized())
        {
            bufferSizeChanged = true;
            currentDamageB.clear();
            currentDamageB.addRect(LRect(0, newSize));
            currentDamage.clear();
            currentDamage.addRect(LRect(0, newSize / current.bufferScale));
        }
        else if (!pendingDamageB.empty() || !pendingDamage.empty())
        {
            if (current.bufferScale == 1)
                damageCalc1();
            else if (current.bufferScale == 2)
                damageCalc2();
            else
                damageCalcN();

            if (current.bufferScale > 1)
                damageMod();
            else
            {
                while (!pendingDamageB.empty())
                {
                    currentDamageB.addRect(pendingDamageB.back());
                    pendingDamageB.pop_back();
                }
            }

            currentDamageB.clip(LRect(0, newSize));
            currentDamage = currentDamageB;
            currentDamage.multiply(1.f/float(current.bufferScale));
        }
    }
};

#endif // LSURFACEPRIVATE_H
