#include <LDNDManager.h>
#include <LSeat.h>
#include <LPointer.h>
#include <LCompositor.h>
#include <LDataSource.h>

using namespace Louvre;

//! [startDragRequest]
void LDNDManager::startDragRequest()
{
    // Let the client start the session only if the origin surface has pointer focus
    if (origin()->hasPointerFocus())
        seat()->pointer()->setDraggingSurface(nullptr);
    else
        cancel();
}
//! [startDragRequest]

//! [cancelled]
void LDNDManager::cancelled()
{
    if (icon())
        icon()->surface()->repaintOutputs();
}
//! [cancelled]
