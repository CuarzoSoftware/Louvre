#include <LCursor.h>
#include <LXCursor.h>
#include <LScene.h>

#include "Surface.h"
#include "DockItem.h"
#include "Global.h"
#include "Output.h"
#include "Dock.h"
#include "Tooltip.h"

DockItem::DockItem(class Surface *surface, Dock *dock) : LTextureView(surface->thumbnailTex, dock->itemsContainer)
{
    surface->minimizedViews.push_back(this);
    this->dock = dock;
    this->surface = surface;
    setBufferScale(4);
    enableScaling(true);
    enableParentOpacity(false);
    enableInput(true);
    enableBlockPointer(false);
}

DockItem::~DockItem()
{
    surface->minimizedViews.remove(this);
}

void DockItem::pointerEnterEvent(const LPoint &localPos)
{
    L_UNUSED(localPos);
    setOpacity(0.8f);

    if (surface->toplevel())
    {
        G::tooltip()->setText(surface->toplevel()->title());
        G::tooltip()->targetView = this;
        dock->update();
    }
    else
        G::tooltip()->hide();
}

void DockItem::pointerLeaveEvent()
{
    setOpacity(1.f);
}

void DockItem::pointerButtonEvent(LPointer::Button button, LPointer::ButtonState state)
{
    if (button != LPointer::Button::Left)
        return;

    if (state == LPointer::Pressed)
        setOpacity(0.7f);
    else
        surface->unminimize(this);
}
