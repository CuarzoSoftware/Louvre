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

    void loadWallpaper();
    void updateTopBar();

    void initializeGL() override;
    void resizeGL() override;
    void moveGL() override;
    void paintGL() override;
    void uninitializeGL() override;

    LSolidColorView *topBarView = nullptr;
    LTextureView *wallpaperView = nullptr;
    Dock *dock = nullptr;
};

#endif // OUTPUT_H
