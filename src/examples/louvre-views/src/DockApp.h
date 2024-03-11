#ifndef DOCKAPP_H
#define DOCKAPP_H

#include "UITextureView.h"

using namespace Louvre;

class App;
class Dock;

class DockApp final : public LTextureView
{
public:
    DockApp(App *app, Dock *dock);
    ~DockApp();

    void pointerEnterEvent(const LPointerEnterEvent &) override;
    void pointerButtonEvent(const LPointerButtonEvent &event) override;

    App *app = nullptr;
    Dock *dock = nullptr;
    UITextureView dot;
};

#endif // DOCKAPP_H
