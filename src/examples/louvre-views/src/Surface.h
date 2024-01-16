#ifndef SURFACE_H
#define SURFACE_H

#include <LSurface.h>
#include <LSurfaceView.h>
#include <LAnimation.h>

#include "Global.h"
#include "DockItem.h"

using namespace Louvre;
using namespace std;

class Toplevel;

class Surface : public LSurface
{
public:
    Surface(LSurface::Params *params);
    ~Surface();

    inline class Toplevel *tl() const {return (class Toplevel*)toplevel();};

    LView *getView() const;

    void parentChanged() override;
    void mappingChanged() override;
    void orderChanged() override;
    void roleChanged() override;
    void bufferSizeChanged() override;
    void minimizedChanged() override;
    void damageChanged() override;

    LTexture *renderThumbnail(LRegion *transRegion = nullptr);
    void unminimize(DockItem *clickedItem);

    bool firstMap = true;
    LSurfaceView *view = nullptr;

    LTextureView *thumbnailFullsizeView = nullptr;
    LTexture *thumbnailFullSizeTex = nullptr;
    LTexture *thumbnailTex = nullptr;
    std::list<DockItem*>minimizedViews;

    LRect minimizeStartRect;
    LAnimation minimizeAnim;
    Output *minimizedOutput = nullptr;
    LRegion minimizedTransRegion;

    LPoint localOutputPos;
    LSize localOutputSize;
    bool outputUnplugHandled = true;

    bool fadedOut = false;

    LTimer *firstMapTimer = nullptr;
};

#endif // SURFACE_H
