#ifndef LSURFACEPRIVATE_H
#define LSURFACEPRIVATE_H

#include <LSurface.h>

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
};

#endif // LSURFACEPRIVATE_H
