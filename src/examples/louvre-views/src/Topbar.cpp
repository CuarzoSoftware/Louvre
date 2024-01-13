#include <LAnimation.h>
#include <LTextureView.h>
#include <LOutputMode.h>
#include "LCursor.h"
#include "Topbar.h"
#include "Global.h"
#include "Compositor.h"
#include "Output.h"
#include "Pointer.h"
#include "TextRenderer.h"

Topbar::Topbar(Output *output) :
    LLayerView(&G::compositor()->overlayLayer),
    background(0.95f, 0.95f, 1.f, 1.f, this),
    logo(G::Logo, &background),
    clock(G::compositor()->clockTexture, &background),
    outputInfo(nullptr, &background),
    oversamplingLabel(G::compositor()->oversamplingLabelTexture, &background),
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

    outputInfo.enableInput(false);
    outputInfo.setBufferScale(2);

    oversamplingLabel.enableCustomColor(true);
    oversamplingLabel.enableInput(false);
    oversamplingLabel.setBufferScale(2);

    appName.setBufferScale(2);
    updateOutputInfo();
    update();
}

Topbar::~Topbar()
{
    output->topbar = nullptr;
}

void Topbar::update()
{
    if (output->scale() != output->fractionalScale() && output->fractionalOversamplingEnabled())
        oversamplingLabel.setCustomColor(0.1f, 0.8f, 0.1f);
    else
        oversamplingLabel.setCustomColor(0.8f, 0.1f, 0.1f);

    setVisible(true);
    setPos(output->pos().x(), output->pos().y());
    background.setSize(output->size().w(), TOPBAR_HEIGHT);
    setSize(output->size().w(), TOPBAR_HEIGHT);
    background.setVisible(true);
    appName.setPos(42, 1 + (background.size().h() - appName.size().h()) / 2);
    clock.setPos(size().w() - clock.size().w() - 8, 1 + (size().h() - clock.size().h()) / 2);
    oversamplingLabel.setPos(clock.nativePos().x() - oversamplingLabel.size().w() - 8, clock.nativePos().y());
    outputInfo.setPos(oversamplingLabel.nativePos().x() - outputInfo.size().w() - 8, oversamplingLabel.nativePos().y());
}

void Topbar::updateOutputInfo()
{
    char info[256];

    sprintf(info, "%s  %dx%d @ %.1fHz @ %.2fx @ %s",
            output->name(),
            output->currentMode()->sizeB().w(),
            output->currentMode()->sizeB().h(),
            Float32(output->currentMode()->refreshRate())/1000,
            output->fractionalScale(),
            G::transformName(output->transform()));

    if (outputInfo.texture())
        delete outputInfo.texture();

    outputInfo.setTexture(G::font()->regular->renderText(info, 22));
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
