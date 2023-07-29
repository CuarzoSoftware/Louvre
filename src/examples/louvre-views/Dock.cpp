#include "Dock.h"
#include "Global.h"
#include "Compositor.h"
#include "Output.h"
#include "DockItem.h"
#include <LTextureView.h>
#include <LCursor.h>
#include <LLog.h>

Dock::Dock(Output *output) : LLayerView(G::compositor()->overlayLayer)
{
    this->output = output;
    enableParentOffset(false);
    enableInput(true);

    dockContainer = new LLayerView(this);

    dockLeft = new LTextureView(G::dockTextures().left, dockContainer);
    dockCenter = new LTextureView(G::dockTextures().center, dockContainer);
    dockRight = new LTextureView(G::dockTextures().right, dockContainer);

    dockLeft->setBufferScale(DOCK_BUFFER_SCALE);
    dockCenter->setBufferScale(DOCK_BUFFER_SCALE);
    dockRight->setBufferScale(DOCK_BUFFER_SCALE);

    dockCenter->enableDstSize(true);

    itemsContainer = new LLayerView(dockContainer);

    // Clone items from another already existing dock
    for (Output *o : G::outputs())
    {
        if (o != output && o->dock)
        {
            for (LView *v : o->dock->itemsContainer->children())
            {
                DockItem *item = (DockItem*)v;
                DockItem *itemClone = new DockItem(item->surface, this);
                itemClone->enableScaling(false);
            }
            break;
        }
    }

    update();
}

Dock::~Dock()
{
    if (anim)
        anim->stop();

    while (!itemsContainer->children().empty())
        delete itemsContainer->children().back();
}

void Dock::update()
{
    setSize(output->size().w(),
            DOCK_HEIGHT + 5);

    setPos(output->rect().x(),
           output->rect().h() - size().h() * visiblePercent);

    Int32 dockWidth = DOCK_PADDING;

    for (LView *it : itemsContainer->children())
    {
        LTextureView *item = (LTextureView*)it;

        item->setPos(dockWidth, DOCK_PADDING);
        dockWidth += item->size().w();

        if (it != itemsContainer->children().back())
            dockWidth += DOCK_SPACING;
    }

    dockWidth += DOCK_PADDING;

    dockLeft->setPos(-DOCK_SHADOW_SIZE,
                     -DOCK_SHADOW_SIZE);

    dockCenter->setPos(dockLeft->nativePos().x() + dockLeft->size().w(),
                       -DOCK_SHADOW_SIZE);

    dockCenter->setDstSize(dockWidth - 2 * DOCK_BORDER_RADIUS,
                           dockCenter->texture()->sizeB().h() / dockCenter->bufferScale());

    dockRight->setPos(dockCenter->nativePos().x() + dockCenter->size().w(),
                      -DOCK_SHADOW_SIZE);


    dockContainer->setSize(dockWidth, DOCK_HEIGHT);
    dockContainer->setPos((size().w() - dockContainer->size().w()) / 2,
                          0);
    itemsContainer->setPos(0, 0);
}

void Dock::show()
{
    if (anim || visiblePercent != 0.f)
        return;

    anim = LAnimation::create(200,
    [this](LAnimation *anim)
    {
        visiblePercent = 1.f - powf(1.f - anim->value(), 2.f);
        update();
        output->repaint();
    },
    [this](LAnimation *)
    {
        anim = nullptr;
    });

    anim->start();
}

void Dock::hide()
{
    if (anim || visiblePercent != 1.f)
        return;

    anim = LAnimation::create(200,
    [this](LAnimation *anim)
    {
        visiblePercent = 1.f - powf(anim->value(), 2.f);
        update();
        output->repaint();
    },
    [this](LAnimation *)
    {
        anim = nullptr;
    });

    anim->start();
}

void Dock::pointerEnterEvent(const LPoint &localPos)
{
    L_UNUSED(localPos);
    show();
}

void Dock::pointerLeaveEvent()
{
    hide();
}
