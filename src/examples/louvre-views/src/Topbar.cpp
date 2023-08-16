#include <LAnimation.h>
#include <LTextureView.h>
#include "LCursor.h"
#include "Topbar.h"
#include "Global.h"
#include "Compositor.h"
#include "Output.h"
#include "Toplevel.h"
#include "src/Pointer.h"
#include "src/ToplevelView.h"

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
    if (anim)
        anim->stop();

    output->topbar = nullptr;
    delete background;
    delete clock;
    delete appName;
}

void Topbar::show()
{
    if (anim || visiblePercent != 0.f)
        return;

    anim = LAnimation::create(
        128,
        [this](LAnimation *a)
        {
            visiblePercent = a->value();
            update();
        },
        [this](LAnimation *)
        {
            visiblePercent = 1.f;
            update();
            anim = nullptr;

            if (!pointerIsOver() && output->fullscreenToplevel)
                LAnimation::oneShot(100, nullptr, [this](LAnimation *){hide();});
        });

    anim->start();
}

void Topbar::hide()
{
    if (anim || visiblePercent != 1.f)
        return;

    anim = LAnimation::create(
        128,
        [this](LAnimation *a)
        {
            visiblePercent = 1.f - a->value();
            update();
        },
        [this](LAnimation *)
        {
            visiblePercent = 0.f;
            update();
            anim = nullptr;
        });

    anim->start();
}

void Topbar::update()
{
    if (output->fullscreenToplevel)
    {
        if (output->fullscreenToplevel->decoratedView)
        {
            setVisible(true);
            setPos(output->pos().x(), output->pos().y() - TOPBAR_HEIGHT);
            background->setVisible(false);
            setSize(output->size().w(), TOPBAR_HEIGHT + TOPLEVEL_TOPBAR_HEIGHT * visiblePercent);
            output->fullscreenToplevel->decoratedView->updateGeometry();
        }
        else
        {
            setVisible(false);
        }
    }
    else
    {
        setVisible(true);
        setPos(output->pos().x(), output->pos().y());
        background->setSize(output->size().w(), TOPBAR_HEIGHT);
        setSize(output->size().w(), TOPBAR_HEIGHT);
        background->setVisible(true);
        appName->setPos(42, 1 + (background ->size().h() - appName->size().h()) / 2);
    }

    clock->setPos(size().w() - clock->size().w() - 8,
                 1 + (size().h() - clock->size().h()) / 2);
}

void Topbar::pointerEnterEvent(const LPoint &localPos)
{
    L_UNUSED(localPos);

    if (!G::pointer()->resizingToplevel() && !G::pointer()->movingToplevel() && !G::pointer()->cursorOwner)
        cursor()->useDefault();

    if (output->fullscreenToplevel && output->fullscreenToplevel->decoratedView)
        show();
}

void Topbar::pointerMoveEvent(const LPoint &)
{
    if (!G::pointer()->resizingToplevel() && !G::pointer()->movingToplevel() && !G::pointer()->cursorOwner)
        cursor()->useDefault();
}

void Topbar::pointerLeaveEvent()
{
    if (output->fullscreenToplevel)
        hide();
}
