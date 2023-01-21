#ifndef LSURFACEPRIVATE_H
#define LSURFACEPRIVATE_H

#include <LSurface.h>

struct Louvre::LSurface::Params
{
    wl_resource *surface;
    LClient *client;
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
    list<LSurface*>::iterator link;

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

    LClient *client                                     = nullptr;
    LSurface *surface                                   = nullptr;
    LSurface *parent                                    = nullptr;
    LSurface *pendingParent                             = nullptr;

    wl_resource *resource = nullptr;

    wl_resource *xdgSurfaceResource                     = nullptr;

    list<wl_resource*> pendingFrameCallbacks;
    wl_resource *frameCallback                          = nullptr;

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

    // Indicates if the surface should be mapped (has a not null buffer)
    bool mapped                                         = false;


};


#endif // LSURFACEPRIVATE_H
