#ifndef DOCKITEM_H
#define DOCKITEM_H

#include <LTextureView.h>

class Surface;
class Dock;

using namespace Louvre;

class DockItem final : public LTextureView
{
public:
    DockItem(class Surface *surface, Dock *dock);
    ~DockItem();

    void pointerEnterEvent(const LPointerEnterEvent &) override;
    void pointerLeaveEvent(const LPointerLeaveEvent &) override;
    void pointerButtonEvent(const LPointerButtonEvent &event) override;

    Dock *dock = nullptr;
    class Surface *surface = nullptr;
};

#endif // DOCKITEM_H
