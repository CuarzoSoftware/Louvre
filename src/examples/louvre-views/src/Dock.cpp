#include "Dock.h"
#include "Global.h"
#include "Compositor.h"
#include "Output.h"
#include "DockItem.h"
#include "Pointer.h"
#include <LTextureView.h>
#include <LCursor.h>
#include <LLog.h>

// Add it to the overlayLayer so it is always on top
Dock::Dock(Output *output) : LLayerView(G::compositor()->overlayLayer)
{
    this->output = output;
    output->dock = this;

    // Detach it from the overlay layer just in case we want to move in the future
    enableParentOffset(false);

    // Enable input so it shows/hides when the cursor is over
    enableInput(true);

    // Allow views behind to get pointer events
    enableBlockPointer(false);

    dockContainer = new LLayerView(this);

    dockLeft = new LTextureView(G::dockTextures().left, dockContainer);
    dockCenter = new LTextureView(G::dockTextures().center, dockContainer);
    dockRight = new LTextureView(G::dockTextures().right, dockContainer);

    dockLeft->setBufferScale(2);
    dockCenter->setBufferScale(2);
    dockRight->setBufferScale(2);

    // Enable dst size so we can use clamping
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

    output->dock = nullptr;
}

void Dock::update()
{
    setSize(output->size().w(),
            DOCK_HEIGHT + 4);

    setPos(output->rect().x(),
           output->rect().h() - size().h() * visiblePercent);

    Int32 dockWidth = DOCK_PADDING;

    for (LView *it : itemsContainer->children())
    {
        LTextureView *item = (LTextureView*)it;

        item->setPos(dockWidth, DOCK_PADDING + (DOCK_ITEM_HEIGHT - item->size().h())/2);
        dockWidth += item->size().w();

        if (it != itemsContainer->children().back())
            dockWidth += DOCK_SPACING;
    }

    dockWidth += DOCK_PADDING;

    dockLeft->setPos(0, 0);
    dockCenter->setPos(dockLeft->nativePos().x() + dockLeft->size().w(), 0);
    dockCenter->setDstSize(dockWidth - 2 * DOCK_BORDER_RADIUS,
                           dockCenter->texture()->sizeB().h() / dockCenter->bufferScale());
    dockRight->setPos(dockCenter->nativePos().x() + dockCenter->size().w(), 0);
    dockContainer->setSize(dockLeft->size().w() + dockCenter->size().w() + dockRight->size().w(), DOCK_HEIGHT);
    dockContainer->setPos((size().w() - dockContainer->size().w()) / 2, - DOCK_SHADOW_SIZE);
    itemsContainer->setPos(DOCK_SHADOW_SIZE, DOCK_SHADOW_SIZE);
}

void Dock::show()
{
    if (anim || visiblePercent != 0.f)
        return;

    dockContainer->setVisible(true);

    anim = LAnimation::create(250,
    [this](LAnimation *anim)
    {
        visiblePercent = 1.f - powf(1.f - anim->value(), 2.f);
        update();
        output->repaint();
    },
    [this](LAnimation *)
    {
        anim = nullptr;

        if (!pointerIsOver())
            LAnimation::oneShot(100, nullptr, [this](LAnimation*){hide();});
    });

    anim->start();
}

void Dock::hide()
{
    if (anim || visiblePercent != 1.f)
        return;

    anim = LAnimation::create(250,
    [this](LAnimation *anim)
    {
        visiblePercent = 1.f - powf(anim->value(), 2.f);
        update();
        output->repaint();
    },
    [this](LAnimation *)
    {
        anim = nullptr;
        dockContainer->setVisible(false);
    });

    anim->start();
}

void Dock::pointerEnterEvent(const LPoint &localPos)
{
    L_UNUSED(localPos);
    showResistanceCount = 0;
}

void Dock::pointerMoveEvent(const LPoint &localPos)
{
    L_UNUSED(localPos);

    if (visiblePercent == 1.f && !G::pointer()->cursorOwner)
        cursor()->useDefault();

    if (showResistanceCount > showResistance)
        show();
    else
        showResistanceCount++;
}

void Dock::pointerLeaveEvent()
{
    showResistanceCount = 0;
    hide();
}
