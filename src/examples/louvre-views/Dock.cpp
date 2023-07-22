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

    dockScene = new LSceneView(LSize(1,1), output->scale(), &dockClipping);
    dockScene->enableParentClipping(true);
    dockScene->setClearColor(0.f, 0.f, 0.f, 0.0f);

    dockBackground = new LSolidColorView(1.f, 1.f, 1.f, 0.8f, dockScene);

    borderRadiusTL = new LTextureView(G::borderRadius()->TL, dockBackground);
    borderRadiusTR = new LTextureView(G::borderRadius()->TR, dockBackground);
    borderRadiusBR = new LTextureView(G::borderRadius()->BR, dockBackground);
    borderRadiusBL = new LTextureView(G::borderRadius()->BL, dockBackground);

    borderRadiusTL->setBlendFunc(GL_ZERO, GL_SRC_ALPHA);
    borderRadiusTR->setBlendFunc(GL_ZERO, GL_SRC_ALPHA);
    borderRadiusBR->setBlendFunc(GL_ZERO, GL_SRC_ALPHA);
    borderRadiusBL->setBlendFunc(GL_ZERO, GL_SRC_ALPHA);

    borderRadiusTL->enableParentOpacity(false);
    borderRadiusTR->enableParentOpacity(false);
    borderRadiusBR->enableParentOpacity(false);
    borderRadiusBL->enableParentOpacity(false);

    borderRadiusTL->setBufferScale(3);
    borderRadiusTR->setBufferScale(3);
    borderRadiusBR->setBufferScale(3);
    borderRadiusBL->setBufferScale(3);

    itemsContainer = new LLayerView(dockBackground);

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
    Int32 backgroundHeight = 2 * DOCK_PADDING + DOCK_ITEM_HEIGHT;

    // Add 2 x margin so it can be detected by cursor
    setSize(LSize(output->size().w(), backgroundHeight + 2 * DOCK_MARGIN));
    setPos(LPoint(output->pos().x(), output->size().h() -  Int32(size().h() * visiblePercent)));

    Int32 backgroundWidth = DOCK_PADDING;

    for (LView *it : itemsContainer->children())
    {
        LTextureView *item = (LTextureView*)it;
        Int32 itemY = (backgroundHeight - item->size().h()) / 2;
        item->setPos(LPoint(backgroundWidth, itemY));
        backgroundWidth += item->size().w();

        if (it != itemsContainer->children().back())
            backgroundWidth += DOCK_SPACING;
    }

    // Default size if no minimized views
    if (itemsContainer->children().empty())
        backgroundWidth += 16;

    backgroundWidth += DOCK_PADDING;

    LSize dockSize = LSize(backgroundWidth, backgroundHeight);

    // Set clipping size and center
    dockClipping.setSize(dockSize);
    dockClipping.setPos(LPoint((size().w() - dockClipping.size().w()) / 2, 5));

    // Set the dock scene buffer size and center
    dockScene->setSizeB(LSize(output->sizeB().w() - (2 * DOCK_MARGIN * output->scale()), dockSize.h() * output->scale()));
    dockScene->setScale(output->scale());
    dockScene->setPos(LPoint((dockClipping.size().w() - dockScene->size().w()) / 2,
                              0));

    dockBackground->setSize(dockSize);
    dockBackground->setPos(LPoint((dockScene->size().w() - dockBackground->size().w()) / 2,
                                   0));

    borderRadiusTL->setPos(LPoint(0, 0));

    borderRadiusTR->setPos(LPoint(dockBackground->size().w() - borderRadiusTR->size().w(),
                                  0));

    borderRadiusBR->setPos(LPoint(dockBackground->size().w() - borderRadiusBR->size().w(),
                                  dockBackground->size().h() - borderRadiusBR->size().h()));

    borderRadiusBL->setPos(LPoint(0,
                                  dockBackground->size().h() - borderRadiusBR->size().h()));

    itemsContainer->setPos(0);
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
