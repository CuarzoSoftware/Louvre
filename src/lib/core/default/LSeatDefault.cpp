#include <LCompositor.h>
#include <LOutput.h>
#include <LSeat.h>
#include <LPointer.h>
#include <LKeyboard.h>
#include <LDataDevice.h>
#include <LCursor.h>
#include <LLog.h>

using namespace Louvre;

//! [initialized]
void LSeat::initialized()
{
    setInputCapabilities(inputBackendCapabilities());
}
//! [initialized]

//! [setSelectionRequest]
bool LSeat::setSelectionRequest(LDataDevice *device, UInt32 serial)
{
    L_UNUSED(serial);

    // Let the client set the clipboard only one of its surfaces has pointer or keyboard focus
    if ((pointer()->focusSurface() && pointer()->focusSurface()->client() == device->client()) ||
       (keyboard()->focusSurface() && keyboard()->focusSurface()->client() == device->client()))
    {
        return true;
    }

    return false;
}
//! [setSelectionRequest]

//! [nativeInputEvent]
void LSeat::nativeInputEvent(void *event)
{
    L_UNUSED(event);
}
//! [nativeInputEvent]

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
    output->setScale(output->dpi() >= 200 ? 2 : 1);

    if (compositor()->outputs().empty())
        output->setPos(0);
    else
        output->setPos(compositor()->outputs().back()->pos() + LPoint(compositor()->outputs().back()->size().w(), 0));

    compositor()->addOutput(output);
    compositor()->repaintAllOutputs();
}
//! [outputPlugged]

//! [outputUnplugged]
void LSeat::outputUnplugged(LOutput *output)
{
    compositor()->removeOutput(output);

    // Organize outputs horizontally and sequentially.

    Int32 totalWidth = 0;

    for (LOutput *o : compositor()->outputs())
    {
        o->setPos(LPoint(totalWidth, 0));
        totalWidth += o->size().w();
    }

    compositor()->repaintAllOutputs();
}
//! [outputUnplugged]
