#include <LPointer.h>
#include <LUtils.h>
#include "DockApp.h"
#include "App.h"
#include "Dock.h"
#include "Global.h"
#include "Tooltip.h"

DockApp::DockApp(App *app, Dock *dock) :
    LTextureView(),
    dot(G::DockDot, this)
{
    setUserData(DockAppType);
    this->app = app;
    this->dock = dock;
    setParent(&dock->appsContainer);
    setTexture(app->texture);
    setBufferScale(2);
    enablePointerEvents(true);
    enableBlockPointer(false);

    dot.setPos((size().w() - dot.size().w()) / 2, size().h() - 2);
    dot.setVisible(false);
    app->dockApps.push_back(this);
    dock->update();
}

DockApp::~DockApp()
{
    app->launchAnimation.stop();
    setParent(nullptr);
    LVectorRemoveOneUnordered(app->dockApps, this);
    dock->update();
}

void DockApp::pointerEnterEvent(const LPointerEnterEvent &)
{
    G::tooltip()->setText(app->name.c_str());
    G::tooltip()->targetView = this;
    dock->update();
}

void DockApp::pointerButtonEvent(const LPointerButtonEvent &event)
{
    if (event.button() == LPointerButtonEvent::Left && event.state() == LPointerButtonEvent::Released)
        app->dockIconClicked();
}
