#include "Tooltip.h"
#include "Global.h"
#include "LTexture.h"
#include "TextRenderer.h"
#include "Compositor.h"

Tooltip::Tooltip()
{
    setVisible(false);
    enableParentOffset(false);
    setParent(&G::compositor()->tooltipsLayer);

    for (Int32 i = 0; i < 8; i++)
    {
        // Clamp textures
        if (i < 4)
            decoration[i].enableDstSize(true);

        decoration[i].setParent(this);
        decoration[i].setBufferScale(2);
        decoration[i].setTexture(G::tooltipTextures().decoration[i]);
    }

    center.setParent(this);
    center.setColor(0.97f, 0.97f, 0.97f);

    arrow.setParent(this);
    arrow.setBufferScale(2);
    arrow.setTexture(G::tooltipTextures().arrow);

    label.setParent(this);
    label.setBufferScale(2);
    label.enableCustomColor(true);
    label.setCustomColor(0.2f, 0.2f, 0.2f);
}

void Tooltip::setText(const char *text)
{
    if (label.texture())
    {
        LTexture *oldTexture = label.texture();
        label.setTexture(nullptr);
        delete oldTexture;
    }

    label.setTexture(G::font()->semibold->renderText(text, 22, 256));
    update();
}

void Tooltip::show(Int32 x, Int32 y)
{
    point.setX(x);
    point.setY(y);

    if (label.texture())
    {
        setVisible(true);
        update();
    }
}

void Tooltip::hide()
{
    setVisible(false);
}

void Tooltip::update()
{
    setSize(label.size() + LSize(16, 12));

    // Center label
    label.setPos((size() - label.size()) / 2);

    center.setPos(CONTAINER_BORDER_RADIUS);
    center.setSize(size() - LSize(2 * CONTAINER_BORDER_RADIUS));

    // Position decoration
    decoration[G::TL].setPos(CONTAINER_OFFSET);
    decoration[G::TR].setPos(size().w() - CONTAINER_BORDER_RADIUS, CONTAINER_OFFSET);
    decoration[G::BR].setPos(size() - LSize(CONTAINER_BORDER_RADIUS));
    decoration[G::BL].setPos(CONTAINER_OFFSET, size().h() - CONTAINER_BORDER_RADIUS);

    decoration[G::T].setPos(CONTAINER_BORDER_RADIUS, CONTAINER_OFFSET);
    decoration[G::T].setDstSize(center.size().w(), decoration[G::T].texture()->sizeB().h() / 2);

    decoration[G::B].setPos(CONTAINER_BORDER_RADIUS, size().h() - CONTAINER_BORDER_RADIUS);
    decoration[G::B].setDstSize(decoration[G::T].size());

    decoration[G::L].setPos(CONTAINER_OFFSET, CONTAINER_BORDER_RADIUS);
    decoration[G::L].setDstSize(decoration[G::L].texture()->sizeB().w() / 2, center.size().h());

    decoration[G::R].setPos(decoration[G::TR].nativePos().x(), CONTAINER_BORDER_RADIUS);
    decoration[G::R].setDstSize(decoration[G::L].size());

    arrow.setPos((size().w() - arrow.size().w()) / 2, size().h());

    setPos(point.x() - (size().w() / 2), point.y() - size().h() - arrow.size().h());
}
