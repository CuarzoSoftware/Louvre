#include <CZ/Louvre/Seat/LIdleListener.h>
#include <CZ/Louvre/LCompositor.h>
#include <CZ/Louvre/Seat/LOutput.h>
#include <CZ/Louvre/Seat/LSeat.h>
#include <CZ/Louvre/Seat/LPointer.h>
#include <CZ/Louvre/Seat/LKeyboard.h>
#include <CZ/Louvre/Seat/LTouch.h>
#include <CZ/Louvre/Cursor/LCursor.h>
#include <CZ/Louvre/LLog.h>
#include <CZ/Core/CZCore.h>

using namespace CZ;

//! [configureOutputsRequest]
bool LSeat::configureOutputsRequest(LClient *client, const std::vector<OutputConfiguration> &configurations)
{
    // All requests accepted by default (unsafe) see LCompositor::globalsFilter().
    CZ_UNUSED(client)

    for (const auto &conf : configurations)
    {
        conf.output.setPos(conf.pos);
        conf.output.setTransform(conf.transform);
        conf.output.setScale(conf.scale);
        conf.output.setMode(conf.mode);

        if (conf.initialized)
            compositor()->addOutput(&conf.output);
        else
            compositor()->removeOutput(&conf.output);
    }

    // Revert changes if the configuration results in zero initialized outputs
    return !compositor()->outputs().empty();
}
//! [configureOutputsRequest]

//! [nativeInputEvent]
void LSeat::nativeInputEvent(void *event)
{
    CZ_UNUSED(event);
}
//! [nativeInputEvent]


void LSeat::inputEvent(const CZInputEvent &e) noexcept
{
    CZ_UNUSED(e)

    /*
     * for (const LIdleListener *idleListener : idleListeners())
     *     idleListener->resetTimer();
     *
     * Resetting all timers each time an event occurs is not CPU-friendly,
     * as multiple events can be triggered in a single main loop iteration.
     * Instead, using the option below (setIsUserIdleHint()) is more efficient.
     */

    /*
     * Setting this flag to false indicates the user wasn't idle during
     * this main loop iteration. If that's the case, Louvre will reset
     * all timers only once at the end of the iteration.
     * The flag is automatically set to true again afterwards.
     */
    setIsUserIdleHint(false);
}


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
    // Probably a VR headset, meant to be leased by clients
    if (output->isNonDesktop())
    {
        output->setLeasable(true);
        return;
    }

    output->setScale(output->dpi() >= 200 ? 2 : 1);

    if (compositor()->outputs().empty())
        output->setPos(SkIPoint(0, 0));
    else
        output->setPos(compositor()->outputs().back()->pos() + SkIPoint(compositor()->outputs().back()->size().width(), 0));

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
        o->setPos(SkIPoint(totalWidth, 0));
        totalWidth += o->size().width();
    }

    compositor()->repaintAllOutputs();
}
//! [outputUnplugged]

//! [inputDevicePluggedEvent]
void LSeat::inputDevicePlugged(std::shared_ptr<CZInputDevice> dev)
{
    CZ_UNUSED(dev);
}
//! [inputDevicePluggedEvent]

//! [inputDeviceUnpluggedEvent]
void LSeat::inputDeviceUnplugged(std::shared_ptr<CZInputDevice> dev)
{
    CZ_UNUSED(dev);
}
//! [inputDeviceUnpluggedEvent]

//! [isIdleStateInhibited]
bool LSeat::isIdleStateInhibited() const
{
    for (LSurface *surface : idleInhibitorSurfaces())
        if (surface->mapped() && !surface->outputs().empty())
            return true;

    return false;
}
//! [isIdleStateInhibited]

//! [onIdleListenerTimeout]
void LSeat::onIdleListenerTimeout(const LIdleListener &listener)
{
    if (isIdleStateInhibited())
        listener.resetTimer();

    /* If the timer is not reset, the client will assume the user is idle. */
}
//! [onIdleListenerTimeout]

//! [event]
bool LSeat::event(const CZEvent &e) noexcept
{
    auto cz { CZCore::Get() };

    if (e.isInputEvent())
    {
        inputEvent((const CZInputEvent&)e);

        if (e.isPointerEvent())
            cz->sendEvent(e, *pointer());
        else if (e.isKeyboardEvent())
            cz->sendEvent(e, *keyboard());
        else if (e.isTouchEvent())
            cz->sendEvent(e, *touch());
    }

    return LFactoryObject::event(e);
}
//! [event]

void LSeat::activeToplevelChanged(LToplevelRole *prev) noexcept
{
    CZ_UNUSED(prev)
}
