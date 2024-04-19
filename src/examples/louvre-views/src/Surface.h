#ifndef SURFACE_H
#define SURFACE_H

#include <LSurface.h>
#include <LSurfaceView.h>
#include <LAnimation.h>
#include <LTimer.h>

#include "Global.h"
#include "DockItem.h"

using namespace Louvre;
using namespace std;

class Toplevel;

class Surface : public LSurface
{
public:
    Surface(const void *params);
    ~Surface();

    inline class Toplevel *tl() const {return (class Toplevel*)toplevel();};

    static class Surface *searchSessionLockParent(Surface *parent);

    LView *getView() const;

    void parentChanged() override;
    void mappingChanged() override;
    void orderChanged() override;
    void roleChanged() override;
    void bufferSizeChanged() override;
    void minimizedChanged() override;
    void damageChanged() override;
    void preferVSyncChanged() override;

    LTexture *renderThumbnail(LRegion *transRegion = nullptr);
    void unminimize(DockItem *clickedItem);

    LAnimation firstMapAnim;
    bool firstMap = true;
    LSurfaceView view;

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

    LTimer firstMapTimer;
};

#endif // SURFACE_H
