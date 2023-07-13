#include "Dock.h"
#include "Shared.h"
#include "Compositor.h"
#include "Output.h"
#include <LTextureView.h>
#include <LCursor.h>

Dock::Dock(Output *output) : LLayerView(comp()->overlayLayer)
{
    m_output = output;
    update();

    LAnimation *anim = new LAnimation(1000, nullptr, [](LAnimation *anim)
    {
        Dock *dock = (Dock*)anim->data();
        dock->hide();
    }, this);

    anim->start();
}

void Dock::update()
{
    Int32 gs = comp()->globalScale();
    Int32 backgroundHeight = 2*m_padding*gs + minimizedItemHeight();

    setNativeSizeC(LSize(m_output->sizeC().w(), 75*gs));
    setNativePosC(LPoint(m_output->posC().x(), (Int32)(m_output->sizeC().h() - 5 - (sizeC().h() - 5)*m_visiblePercent)));

    Int32 backgroundWidth = m_padding*gs;

    for (LView *it : background.children())
    {
        LTextureView *item = (LTextureView*)it;
        Int32 itemY = backgroundHeight/2 - item->sizeC().h()/2;
        item->setNativePosC(LPoint(backgroundWidth, itemY));
        backgroundWidth += item->sizeC().w();

        if (it != background.children().back())
            backgroundWidth += m_spacing*gs;
    }

    backgroundWidth += m_padding*gs;

    background.setNativeSizeC(LSize(backgroundWidth, backgroundHeight));
    background.setNativePosC(LPoint(sizeC().w()/2 - backgroundWidth/2, 10));
}

void Dock::show()
{
    if (m_visiblePercent != 0.f)
        return;

    LAnimation *anim = new LAnimation(100,
    [](LAnimation *anim)->bool
    {
        Dock *dock = (Dock*)anim->data();
        dock->m_visiblePercent = anim->value();
        dock->update();
        dock->m_output->repaint();
        return true;
    },
    nullptr, this);

    anim->start();
}

void Dock::hide()
{
    if (m_visiblePercent != 1.f)
        return;

    LAnimation *anim = new LAnimation(100,
    [](LAnimation *anim)->bool
    {
        Dock *dock = (Dock*)anim->data();
        dock->m_visiblePercent = 1.f - anim->value();
        dock->update();
        dock->m_output->repaint();
        return true;
    },
    nullptr, this);

    anim->start();
}

void Dock::handleCursorMovement()
{
    LRect r = LRect(posC(), sizeC());

    if (r.containsPoint(cursor()->posC()))
        show();
    else
        hide();
}
