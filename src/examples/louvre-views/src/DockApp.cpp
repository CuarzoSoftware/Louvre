#include "DockApp.h"
#include "App.h"
#include "Dock.h"
#include "Global.h"
#include "Tooltip.h"

DockApp::DockApp(App *app, Dock *dock) :
    LTextureView(),
    dot(G::DockDot, this)
{
    this->app = app;
    this->dock = dock;
    setParent(&dock->appsContainer);
    setTexture(app->texture);
    setBufferScale(2);
    enableInput(true);
    enableBlockPointer(false);

    dot.setPos((size().w() - dot.size().w()) / 2, size().h() - 2);
    dot.setVisible(false);
    app->dockApps.push_back(this);
    appLink = std::prev(app->dockApps.end());
    dock->update();
}

DockApp::~DockApp()
{
    app->launchAnimation.stop();
    setParent(nullptr);
    app->dockApps.erase(appLink);
    dock->update();
}

void DockApp::pointerEnterEvent(const LPoint &)
{
    G::tooltip()->setText(app->name.c_str());
    G::tooltip()->targetView = this;
    dock->update();
}

void DockApp::pointerButtonEvent(LPointer::Button button, LPointer::ButtonState state)
{
    if (button == LPointer::Left && state == LPointer::Released)
        app->clicked();
}
