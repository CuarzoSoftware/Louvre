#ifndef SURFACE_H
#define SURFACE_H

#include <LSurface.h>
#include <LSurfaceView.h>

#include "Compositor.h"
#include "Global.h"
#include "DockItem.h"

using namespace Louvre;
using namespace std;

class Surface : public LSurface
{
public:
    Surface(LSurface::Params *params);
    ~Surface();

    LView *getView() const;

    void parentChanged() override;
    void mappingChanged() override;
    void orderChanged() override;
    void roleChanged() override;
    void bufferSizeChanged() override;
    void minimizedChanged() override;

    LTexture *renderThumbnail();
    void unminimize(DockItem *clickedItem);

    bool firstMap = true;
    LSurfaceView *view = nullptr;

    LTextureView *thumbnailFullsizeView = nullptr;
    LTexture *thumbnailFullSizeTex = nullptr;
    LTexture *thumbnailTex = nullptr;
    std::list<DockItem*>minimizedViews;

    LRect minimizeStartRect;
    LAnimation *minimizeAnim = nullptr;
    Output *minimizedOutput = nullptr;
};

#endif // SURFACE_H
