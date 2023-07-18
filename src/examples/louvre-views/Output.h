#ifndef OUTPUT_H
#define OUTPUT_H

#include <LOutput.h>
#include <LSolidColorView.h>
#include <LRenderBuffer.h>
#include <LSceneView.h>

using namespace Louvre;

class Compositor;
class Dock;

class Output : public LOutput
{
public:
    Output();
    Compositor *compositor() const;
    void loadWallpaper();
    void initializeGL() override;
    void resizeGL() override;
    void paintGL() override;
    void uninitializeGL() override;

    LTextureView *wallpaperView = nullptr;
    Dock *dock = nullptr;
    LSceneView *sView = nullptr;

};

#endif // OUTPUT_H
