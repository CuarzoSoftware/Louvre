#ifndef DOCKITEM_H
#define DOCKITEM_H

#include <LTextureView.h>

class Surface;
class Dock;

using namespace Louvre;

class DockItem : public LTextureView
{
public:
    DockItem(class Surface *surface, Dock *dock);
    ~DockItem();

    void pointerEnterEvent(const LPoint &) override;
    void pointerLeaveEvent() override;
    void pointerButtonEvent(LPointer::Button button, LPointer::ButtonState state) override;

    Dock *dock = nullptr;
    class Surface *surface = nullptr;
};

#endif // DOCKITEM_H
