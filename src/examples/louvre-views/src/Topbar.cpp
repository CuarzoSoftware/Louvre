#include <LAnimation.h>
#include <LTextureView.h>
#include "LCursor.h"
#include "Topbar.h"
#include "Global.h"
#include "Compositor.h"
#include "Output.h"
#include "src/Pointer.h"

Topbar::Topbar(Output *output) :
    LLayerView(&G::compositor()->overlayLayer),
    background(0.95f, 0.95f, 1.f, 1.f, this),
    logo(G::Logo, &background),
    clock(nullptr, &background),
    appName(G::textures()->defaultTopbarAppName, &background)
{
    this->output = output;
    output->topbar = this;
    enableParentOffset(false);
    enableInput(true);
    enableBlockPointer(false);

    background.enableInput(false);
    background.enableBlockPointer(false);
    background.setPos(0, 0);

    logo.enableCustomColor(true);
    logo.setCustomColor(0.1f, 0.1f, 0.1f);
    logo.setPos(12, 5);

    clock.enableInput(false);
    clock.setBufferScale(2);

    appName.setBufferScale(2);
}

Topbar::~Topbar()
{
    output->topbar = nullptr;
}

void Topbar::update()
{
    setVisible(true);
    setPos(output->pos().x(), output->pos().y());
    background.setSize(output->size().w(), TOPBAR_HEIGHT);
    setSize(output->size().w(), TOPBAR_HEIGHT);
    background.setVisible(true);
    appName.setPos(42, 1 + (background.size().h() - appName.size().h()) / 2);
    clock.setPos(size().w() - clock.size().w() - 8, 1 + (size().h() - clock.size().h()) / 2);
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
