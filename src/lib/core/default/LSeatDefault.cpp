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
bool LSeat::setSelectionRequest(LDataDevice *device)
{
    // Let the client set the clipboard only if one of its surfaces has pointer or keyboard focus
    return (pointer()->focus() && pointer()->focus()->client() == device->client()) ||
           (keyboard()->focus() && keyboard()->focus()->client() == device->client());
}
//! [setSelectionRequest]

//! [nativeInputEvent]
void LSeat::nativeInputEvent(void *event)
{
    L_UNUSED(event);
}
//! [nativeInputEvent]

//! [enabledChanged]
void LSeat::enabledChanged()
{
    if (!enabled())
        return;

    cursor()->setVisible(false);
    cursor()->setVisible(true);
    cursor()->move(1, 1);
    compositor()->repaintAllOutputs();
}
//! [enabledChanged]

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
