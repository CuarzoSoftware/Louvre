#ifndef DOCKAPP_H
#define DOCKAPP_H

#include <LTextureView.h>

using namespace Louvre;

class App;
class Dock;

class DockApp : public LTextureView
{
public:
    DockApp(App *app, Dock *dock);
    ~DockApp();

    void pointerButtonEvent(LPointer::Button button, LPointer::ButtonState state) override;

    std::list<DockApp*>::iterator appLink;
    App *app = nullptr;
    Dock *dock = nullptr;
    LTextureView *dot = nullptr;
};

#endif // DOCKAPP_H
