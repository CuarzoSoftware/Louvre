#include "Tooltip.h"
#include "Global.h"
#include "LTexture.h"
#include "TextRenderer.h"
#include "Compositor.h"

Tooltip::Tooltip() :
    decoT(G::TooltipT, this),
    decoR(G::TooltipR, this),
    decoB(G::TooltipB, this),
    decoL(G::TooltipL, this),
    decoTL(G::TooltipTL, this),
    decoTR(G::TooltipTR, this),
    decoBR(G::TooltipBR, this),
    decoBL(G::TooltipBL, this),
    arrow(G::TooltipArrow, this)
{
    setVisible(false);
    enableParentOffset(false);
    setParent(&G::compositor()->tooltipsLayer);

    center.setParent(this);
    center.setColor(0.97f, 0.97f, 0.97f);

    label.setParent(this);
    label.setBufferScale(2);
    label.enableCustomColor(true);
    label.setCustomColor(0.2f, 0.2f, 0.2f);
}

void Tooltip::setText(const char *text)
{
    if (label.texture())
        delete label.texture();

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
    label.setTexture(nullptr);
}

void Tooltip::update()
{
    setSize(label.size() + LSize(16, 12));

    // Center label
    label.setPos((size() - label.size()) / 2);

    center.setPos(CONTAINER_BORDER_RADIUS);
    center.setSize(size() - LSize(2 * CONTAINER_BORDER_RADIUS));

    // Position decoration
    decoTL.setPos(CONTAINER_OFFSET);
    decoTR.setPos(size().w() - CONTAINER_BORDER_RADIUS, CONTAINER_OFFSET);
    decoBR.setPos(size() - LSize(CONTAINER_BORDER_RADIUS));
    decoBL.setPos(CONTAINER_OFFSET, size().h() - CONTAINER_BORDER_RADIUS);

    decoT.setPos(CONTAINER_BORDER_RADIUS, CONTAINER_OFFSET);
    decoT.setDstSize(center.size().w(), decoT.nativeSize().h());

    decoB.setPos(CONTAINER_BORDER_RADIUS, size().h() - CONTAINER_BORDER_RADIUS);
    decoB.setDstSize(decoT.size());

    decoL.setPos(CONTAINER_OFFSET, CONTAINER_BORDER_RADIUS);
    decoL.setDstSize(decoL.nativeSize().w(), center.size().h());

    decoR.setPos(decoTR.nativePos().x(), CONTAINER_BORDER_RADIUS);
    decoR.setDstSize(decoL.size());

    arrow.setPos((size().w() - arrow.size().w()) / 2, size().h() - 1);

    setPos(point.x() - (size().w() / 2), point.y() - size().h() - arrow.size().h());
}

bool Tooltip::nativeMapped() const
{
    return label.texture() != nullptr;
}
