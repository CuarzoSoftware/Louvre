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

    struct State
    {
        LBaseSurfaceRole *role                          = nullptr;
        wl_resource *buffer                             = nullptr;
        Int32 bufferScale                               = 1;
    };

    // Link in LSurfaces compositor list
    std::list<LSurface*>::iterator compositorLink, clientLink;

    LRegion pendingInputRegionS;
    LRegion currentInputRegionS;
    LRegion currentInputRegionC;

    LRegion pendingOpaqueRegionS;
    LRegion currentOpaqueRegionS;
    LRegion currentOpaqueRegionC;

    LRegion pendingTranslucentRegionS;
    LRegion currentTranslucentRegionS;
    LRegion currentTranslucentRegionC;

    LSize currentSizeB;
    LSize currentSizeS;
    LSize currentSizeC;

    std::vector<LRect> pendingDamagesB;
    std::vector<LRect> pendingDamagesS;
    LRegion currentDamagesB;
    LRegion currentDamagesC;

    LTexture *texture                                   = nullptr;
    LTexture *textureBackup;

    std::list<LSurface*> children;
    std::list<LSurface*> pendingChildren;

    // Buffer
    void setBufferScale(Int32 scale);
    void globalScaleChanged(Int32 oldScale, Int32 newScale);

    LSurface *parent                                    = nullptr;
    LSurface *pendingParent                             = nullptr;
    std::list<LSurface*>::iterator parentLink;
    std::list<LSurface*>::iterator pendingParentLink;

    Wayland::RSurface *surfaceResource = nullptr;

    std::list<Wayland::RCallback*>frameCallbacks;

    State current, pending;

    std::list<LOutput*> outputs;

    bool damaged                                        = false;
    UInt32 damageId;
    LPoint posC;
    bool minimized                                      = false;
    bool receiveInput                                   = true;
    bool opaqueRegionChanged                            = false;
    bool inputRegionChanged                             = false;
    bool inputRegionIsInfinite                          = true;
    bool damagesChanged                                 = false;
    bool bufferSizeChanged                              = false;
    bool bufferReleased                                 = true;
    bool atached                                        = false;
    // Indicates if the surface should be mapped (has a not null buffer)
    bool mapped                                         = false;

    // Presentation feedback
    std::list<WpPresentationTime::RWpPresentationFeedback*> wpPresentationFeedbackResources;
    void sendPresentationFeedback(LOutput *output, timespec &ns);
};

#endif // LSURFACEPRIVATE_H
