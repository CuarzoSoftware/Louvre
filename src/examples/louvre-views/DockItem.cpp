#include <LCursor.h>
#include <LXCursor.h>
#include <LScene.h>

#include "Surface.h"
#include "DockItem.h"
#include "Global.h"
#include "Output.h"
#include "Dock.h"

DockItem::DockItem(class Surface *surface, Dock *dock) : LTextureView(surface->thumbnailTex, dock->itemsContainer)
{
    surface->minimizedViews.push_back(this);
    this->dock = dock;
    this->surface = surface;
    setBufferScale(surface->view->bufferScale());
    enableScaling(true);
    enableParentOpacity(false);
    enableInput(true);
}

DockItem::~DockItem()
{
    surface->minimizedViews.remove(this);
}

void DockItem::pointerEnterEvent(const LPoint &localPos)
{
    L_UNUSED(localPos);
    setOpacity(0.9f);

    if (G::cursors().handCursor != nullptr)
        cursor()->setTextureB(G::cursors().handCursor->texture(),
                              G::cursors().handCursor->hotspotB());

    G::scene()->enableHandleWaylandPointerEvents(false);
}

void DockItem::pointerLeaveEvent()
{
    setOpacity(1.f);
    G::scene()->enableHandleWaylandPointerEvents(true);
}

void DockItem::pointerButtonEvent(LPointer::Button button, LPointer::ButtonState state)
{
    if (button != LPointer::Button::Left)
        return;

    if (state == LPointer::Pressed)
    {
        setOpacity(0.85f);
    }
    else
    {
        G::scene()->enableHandleWaylandPointerEvents(true);
        surface->unminimize(this);
    }
}
