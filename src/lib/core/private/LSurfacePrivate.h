#ifndef LSURFACEPRIVATE_H
#define LSURFACEPRIVATE_H

#include <LSurface.h>

struct Louvre::LSurface::Params
{
    Protocols::Wayland::SurfaceResource *surfaceResource;
};

class Louvre::LSurface::LSurfacePrivate
{
public:

    LSurfacePrivate()                                   = default;
    ~LSurfacePrivate()                                  = default;

    LSurfacePrivate(const LSurfacePrivate&)             = delete;
    LSurfacePrivate& operator= (const LSurfacePrivate&) = delete;

    void setParent(LSurface *parent);
    void removeChild(LSurface *child);
    void setMapped(bool state);
    void setPendingRole(LBaseSurfaceRole *role);
    void applyPendingRole();
    void applyPendingChildren();
    bool bufferToTexture();
    void notifyPosUpdateToChildren(LSurface *surface);

    struct State
    {
        LBaseSurfaceRole *role                          = nullptr;
        wl_resource *buffer                             = nullptr;
        Int32 bufferScale                               = 1;
    };

    // Link in LSurfaces compositor list
    list<LSurface*>::iterator compositorLink, clientLink;

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

    LRegion pendingDamagesB;
    LRegion pendingDamagesS;
    LRegion currentDamagesB;
    LRegion currentDamagesC;

    LTexture *texture                                   = nullptr;

    list<LSurface*> children;
    list<LSurface*> pendingChildren;

    // Buffer
    void setBufferScale(Int32 scale);
    void globalScaleChanged(Int32 oldScale, Int32 newScale);

    LSurface *parent                                    = nullptr;
    LSurface *pendingParent                             = nullptr;

    Protocols::Wayland::SurfaceResource *surfaceResource = nullptr;
    wl_resource *xdgSurfaceResource                     = nullptr;

    wl_resource* frameCallback                          = nullptr;

    State current, pending;

    list<LOutput*> outputs;

    bool damaged                                        = false;

    LPoint posC;
    bool minimized                                      = false;
    bool receiveInput                                   = true;
    bool opaqueRegionChanged                            = false;
    bool inputRegionChanged                             = false;
    bool inputRegionIsInfinite                          = true;
    bool damagesChanged                                 = false;
    bool bufferSizeChanged                              = false;
    bool bufferReleased                                 = true;

    // Indicates if the surface should be mapped (has a not null buffer)
    bool mapped                                         = false;



    // Presentation feedback
    list<wl_resource*> presentationFeedback;
    list<wl_resource*> presentationOutputResources;
    LOutput *presentationOutput = nullptr;
    void sendPresentationFeedback(LOutput *output, timespec &ns);


};


#endif // LSURFACEPRIVATE_H
