#include <LDNDManager.h>
#include <LSeat.h>
#include <LPointer.h>
#include <LCompositor.h>
#include <LDataSource.h>

using namespace Louvre;

//! [startDragRequest]
void LDNDManager::startDragRequest()
{
    // Let the client set the clipboard only if one of its surfaces has pointer or keyboard focus
    if ((seat()->pointer()->focusSurface() && seat()->pointer()->focusSurface()->client() == source()->client()) ||
       (seat()->keyboard()->focusSurface() && seat()->keyboard()->focusSurface()->client() == source()->client()))
    {
        seat()->pointer()->setDraggingSurface(nullptr);
    }
    else
    {
        cancel();
    }
}
//! [startDragRequest]

//! [cancelled]
void LDNDManager::cancelled()
{
    if (icon())
        icon()->surface()->repaintOutputs();
}
//! [cancelled]
