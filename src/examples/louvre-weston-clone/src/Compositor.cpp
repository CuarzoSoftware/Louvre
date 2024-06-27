#include <LSeat.h>
#include <LKeyboard.h>
#include <LOutput.h>
#include <LOutputMode.h>
#include <LOpenGL.h>

#include "Compositor.h"
#include "Output.h"
#include "Surface.h"
#include "ToplevelRole.h"
#include "Pointer.h"
#include "Seat.h"
#include "Popup.h"

void Compositor::initialized()
{
    pointerCursor.reset(LXCursor::load("hand2"));
    terminalIconTexture.reset(LOpenGL::loadTexture(compositor()->defaultAssetsPath() / "terminal.png"));
    LCompositor::initialized();
}

void Compositor::uninitialized()
{
    pointerCursor.reset();
    terminalIconTexture.reset();
    clock.reset();
}

LFactoryObject *Compositor::createObjectRequest(LFactoryObject::Type type, const void *params)
{
    if (type == LFactoryObject::Type::LSurface)
        return new Surface(params);

    if (type == LFactoryObject::Type::LToplevelRole)
        return new ToplevelRole(params);

    if (type == LFactoryObject::Type::LPopupRole)
        return new Popup(params);

    if (type == LFactoryObject::Type::LOutput)
        return new Output(params);

    if (type == LFactoryObject::Type::LSeat)
        return new Seat(params);

    if (type == LFactoryObject::Type::LPointer)
        return new Pointer(params);

    return nullptr;
}

void Compositor::onAnticipatedObjectDestruction(LFactoryObject *object)
{
    if (object->factoryObjectType() == LFactoryObject::Type::LSurface)
    {
        Surface *surface {static_cast<Surface*>(object)};
        for (Output *output : (const std::vector<Output*>&)outputs())
            if (surface == output->fullscreenSurface)
                output->fullscreenSurface = nullptr;
    }
}
