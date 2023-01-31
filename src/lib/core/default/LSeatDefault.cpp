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
    if((pointer()->focusSurface() && pointer()->focusSurface()->client() == device->client()) ||
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
    if(cursor())
        cursor()->useDefault();
}
//! [seatEnabled]

//! [seatDisabled]
void LSeat::seatDisabled()
{
    /* No default implementation */
}
//! [seatDisabled]
