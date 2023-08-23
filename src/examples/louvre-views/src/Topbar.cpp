#include <LAnimation.h>
#include <LTextureView.h>
#include "LCursor.h"
#include "Topbar.h"
#include "Global.h"
#include "Compositor.h"
#include "Output.h"
#include "src/Pointer.h"

Topbar::Topbar(Output *output) : LLayerView(&G::compositor()->overlayLayer)
{
    this->output = output;
    output->topbar = this;
    enableParentOffset(false);
    enableInput(true);
    enableBlockPointer(false);

    background = new LSolidColorView(0.95f, 0.95f, 1.f, 1.f, this);
    background->enableInput(false);
    background->enableBlockPointer(false);
    background->setPos(0, 0);

    logo = new LTextureView(G::toplevelTextures().logo, background);
    logo->setBufferScale(2);
    logo->enableCustomColor(true);
    logo->setCustomColor(0.1f, 0.1f, 0.1f);
    logo->setPos(12, 5);

    // Todo USE textures of another topbar if exists

    appName = new LTextureView(G::toplevelTextures().defaultTopbarAppName, background);
    appName->setBufferScale(2);

    clock = new LTextureView(nullptr, background);
    clock->enableInput(false);
    clock->setBufferScale(2);
}

Topbar::~Topbar()
{
    output->topbar = nullptr;
    delete background;
    delete clock;
    delete appName;
}

void Topbar::update()
{
    setVisible(true);
    setPos(output->pos().x(), output->pos().y());
    background->setSize(output->size().w(), TOPBAR_HEIGHT);
    setSize(output->size().w(), TOPBAR_HEIGHT);
    background->setVisible(true);
    appName->setPos(42, 1 + (background ->size().h() - appName->size().h()) / 2);
    clock->setPos(size().w() - clock->size().w() - 8,
                 1 + (size().h() - clock->size().h()) / 2);
}

void Topbar::pointerEnterEvent(const LPoint &localPos)
{
    L_UNUSED(localPos);

    if (!G::pointer()->resizingToplevel() && !G::pointer()->movingToplevel() && !G::pointer()->cursorOwner)
        cursor()->useDefault();
}

void Topbar::pointerMoveEvent(const LPoint &)
{
    if (!G::pointer()->resizingToplevel() && !G::pointer()->movingToplevel() && !G::pointer()->cursorOwner)
        cursor()->useDefault();
}
