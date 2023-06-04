#include "LCompositor.h"
#include "LLog.h"
#include "LOutput.h"
#include <LSeat.h>
#include <LPointer.h>
#include <LKeyboard.h>
#include <LDataDevice.h>
#include <LCursor.h>

using namespace Louvre;

//! [initialized]
void LSeat::initialized()
{
    setCapabilities(backendCapabilities());
}
//! [initialized]

//! [setSelectionRequest]
bool LSeat::setSelectionRequest(LDataDevice *device)
{
    // Let the client set the clipboard only one of its surfaces has pointer or keyboard focus
    if ((pointer()->focusSurface() && pointer()->focusSurface()->client() == device->client()) ||
       (keyboard()->focusSurface() && keyboard()->focusSurface()->client() == device->client()))
    {
        return true;
    }

    return false;
}
//! [setSelectionRequest]

//! [backendNativeEvent]
void LSeat::backendNativeEvent(void *event)
{
    L_UNUSED(event);
}
//! [backendNativeEvent]

//! [seatEnabled]
void LSeat::seatEnabled()
{
    if (cursor())
    {
        cursor()->setVisible(false);
        cursor()->setVisible(true);
    }

    compositor()->repaintAllOutputs();
}
//! [seatEnabled]

//! [seatDisabled]
void LSeat::seatDisabled()
{
    /* No default implementation */
}
//! [seatDisabled]

//! [outputPlugged]
void LSeat::outputPlugged(LOutput *output)
{
    LLog::debug("Output %s connected.", output->name());

    output->setScale(output->dpi() >= 120 ? 2 : 1);

    LCompositor::compositor()->addOutput(output);

    // Organize outputs horizontally and sequentially.

    Int32 totalWidth = 0;

    for (LOutput *o : compositor()->outputs())
    {
        o->setPosC(LPoint(totalWidth,0));

        totalWidth += o->sizeC().w();
    }

    compositor()->repaintAllOutputs();
}
//! [outputPlugged]

//! [outputUnplugged]
void LSeat::outputUnplugged(LOutput *output)
{
    compositor()->removeOutput(output);

    // Organize outputs horizontally and sequentially.

    Int32 totalWidth = 0;

    if (!outputs()->empty())
    {
        for (LOutput *o : compositor()->outputs())
        {
            o->setPosC(LPoint(totalWidth,0));
            totalWidth += o->sizeC().w();
        }

        compositor()->repaintAllOutputs();
    }
}
//! [outputUnplugged]
