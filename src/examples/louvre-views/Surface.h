#ifndef SURFACE_H
#define SURFACE_H

#include <LSurface.h>
#include <LSurfaceView.h>
#include <Compositor.h>
#include <Global.h>

using namespace Louvre;
using namespace std;

class Surface : public LSurface
{
public:
    Surface(LSurface::Params *params);
    ~Surface();
    Compositor *compositor() const;

    void mappingChanged() override;
    void orderChanged() override;
    void roleChanged() override;
    void bufferSizeChanged() override;
    void minimizedChanged() override;

    bool firstMap = true;
    LSurfaceView *view = nullptr;

    LPoint posBeforeMinimized;
    LTexture *minimizedTexture;
    std::list<LTextureView*>minimizedViews;

    LAnimation *minimizeAnim = nullptr;
};

#endif // SURFACE_H
