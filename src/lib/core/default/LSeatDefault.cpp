#include <LCompositor.h>
#include <LOutput.h>
#include <LSeat.h>
#include <LPointer.h>
#include <LKeyboard.h>
#include <LCursor.h>
#include <LLog.h>

using namespace Louvre;

//! [initialized]
void LSeat::initialized()
{
    setInputCapabilities(inputBackendCapabilities());
}
//! [initialized]

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

    Int32 totalWidth { 0 };

    for (LOutput *o : compositor()->outputs())
    {
        o->setPos(LPoint(totalWidth, 0));
        totalWidth += o->size().w();
    }

    compositor()->repaintAllOutputs();
}
//! [outputUnplugged]

//! [inputDevicePlugged]
void LSeat::inputDevicePlugged(LInputDevice *device)
{
    L_UNUSED(device);
    setInputCapabilities(inputBackendCapabilities());
}
//! [inputDevicePlugged]

//! [inputDeviceUnplugged]
void LSeat::inputDeviceUnplugged(LInputDevice *device)
{
    L_UNUSED(device);
    setInputCapabilities(inputBackendCapabilities());
}
//! [inputDeviceUnplugged]
