#include <LOutput.h>
#include <LPainter.h>
#include <LCompositor.h>
#include <LSurface.h>
#include <LSeat.h>
#include <LDNDManager.h>
#include <LDNDIconRole.h>
#include <LCursor.h>

using namespace Louvre;

//! [initializeGL]
void LOutput::initializeGL()
{
    // Sets the background color to white
    painter()->setClearColor(1.f, 1.f, 1.f, 1.f);
}
//! [initializeGL]

//! [paintGL]
void LOutput::paintGL()
{
    LPainter *p = painter();

    p->clearScreen();

    // Check if surface moved under cursor
    if (seat()->pointer()->surfaceAt(cursor()->pos()) != seat()->pointer()->focusSurface())
        seat()->pointer()->pointerPosChangeEvent(
            cursor()->pos().x(),
            cursor()->pos().y());

    if (seat()->dndManager()->icon())
        seat()->dndManager()->icon()->surface()->raise();

    // Draws every surface
    for (LSurface *s : compositor()->surfaces())
    {
        // Skip some surfaces
        if (!s->mapped() || s->minimized() || s->cursorRole())
        {
            s->requestNextFrame();
            continue;
        }

        // Current surface rect
        LRect currentRect = LRect(
            s->rolePos(),  // Role pos in compositor coords
            s->size());    // Surface size in compositor coords

        // Calc which outputs intersects the surface
        for (LOutput *o : compositor()->outputs())
        {
            if (o->rect().intersects(currentRect))
                s->sendOutputEnterEvent(o);
            else
                s->sendOutputLeaveEvent(o);
        }

        // Draws the surface
        p->drawTexture(
            s->texture(),        // Surface texture
            LRect(0, s->sizeB()),// The entire texture size
            currentRect);        // The destination rect on screen

        // Notifies the client that it can render the next frame 
        s->requestNextFrame();
    }

    // Manualy draw the cursor if hardware composition is not supported
    if (!cursor()->hasHardwareSupport(this))
    {
        p->drawTexture(
            cursor()->texture(),                   
            LRect(0, cursor()->texture()->sizeB()),
            cursor()->rect());
    }
}
//! [paintGL]

//! [resizeGL]
void LOutput::resizeGL()
{
    repaint();
}
//! [resizeGL]

//! [moveGL]
void LOutput::moveGL()
{
    repaint();
}
//! [moveGL]

//! [uninitializeGL]
void LOutput::uninitializeGL()
{
    /* No default implementation */
}
//! [uninitializeGL]
