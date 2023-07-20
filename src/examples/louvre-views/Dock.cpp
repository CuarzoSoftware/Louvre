#include "Dock.h"
#include "Global.h"
#include "Compositor.h"
#include "Output.h"
#include <LTextureView.h>
#include <LCursor.h>
#include <LLog.h>

Dock::Dock(Output *output) : LLayerView(G::compositor()->overlayLayer)
{
    m_output = output;
    enableParentOffset(false);
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

    itemsContainer = new LLayerView(dockBackground);

    update();

    LAnimation::oneShot(1000, nullptr, [this](LAnimation *)
    {
        hide();
    });
}

void Dock::update()
{
    Int32 backgroundHeight = 2 * DOCK_PADDING + DOCK_ITEM_HEIGHT;

    // Add 5 so it can be detected by cursor
    setSize(LSize(output()->size().w(), backgroundHeight + 5));

    setPos(LPoint(output()->pos().x(),
                         output()->size().h() -  Int32(size().h() * m_visiblePercent)));

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
        backgroundWidth += 128;

    backgroundWidth += DOCK_PADDING;

    LSize dockSize = LSize(backgroundWidth, backgroundHeight);

    // Set clipping size and center
    dockClipping.setSize(dockSize);
    dockClipping.setPos(LPoint((size().w() - dockClipping.size().w()) / 2, 0));

    // Set the dock scene buffer size and center
    dockScene->setSizeB(LSize(output()->sizeB().w(), dockSize.h() * output()->scale()));
    dockScene->setScale(output()->scale());
    dockScene->setPos(LPoint((dockClipping.size().w() - dockScene->size().w()) / 2,
                              0));

    dockBackground->setSize(dockSize);
    dockBackground->setPos(LPoint( (dockScene->size().w() - dockBackground->size().w()) / 2,
                                   0));

    borderRadiusTL->setBufferScale(2);
    borderRadiusTL->setPos(LPoint(0, 0));

    borderRadiusTR->setBufferScale(2);
    borderRadiusTR->setPos(LPoint(dockBackground->size().w() - borderRadiusTR->size().w(),
                                  0));

    borderRadiusBR->setBufferScale(2);
    borderRadiusBR->setPos(LPoint(dockBackground->size().w() - borderRadiusBR->size().w(),
                                  dockBackground->size().h() - borderRadiusBR->size().h()));

    borderRadiusBL->setBufferScale(2);
    borderRadiusBL->setPos(LPoint(0,
                                  dockBackground->size().h() - borderRadiusBR->size().h()));

    itemsContainer->setPos(0);
}

void Dock::show()
{
    if (m_visiblePercent != 0.f)
        return;

    LAnimation::oneShot(200,
    [this](LAnimation *anim)
    {
        m_visiblePercent = 1.f - powf(1.f - anim->value(), 2.f);
        update();
        m_output->repaint();
    });
}

void Dock::hide()
{
    if (m_visiblePercent != 1.f)
        return;

    LAnimation::oneShot(200,
    [this](LAnimation *anim)
    {
        m_visiblePercent = 1.f - anim->value()*anim->value();
        update();
        m_output->repaint();
        return true;
    });
}

void Dock::handleCursorMovement()
{
    LRect r = LRect(pos(), size());

    if (r.containsPoint(cursor()->pos()))
        show();
    else
        hide();
}

Output *Dock::output() const
{
    return m_output;
}
