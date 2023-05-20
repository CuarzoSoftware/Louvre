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

    paintGL();
}
//! [initializeGL]

//! [paintGL]
void LOutput::paintGL()
{
    LPainter *p = painter();

    p->clearScreen();

    // Draws every surface
    for(LSurface *s : compositor()->surfaces())
    {
        // Skip some surfaces
        if(!s->mapped() || s->dndIcon() || s->cursor())
        {
            s->requestNextFrame();
            continue;
        }

        // Current surface rect
        LRect currentRect = LRect(
            s->rolePosC(),  // Role pos in compositor coords
            s->sizeC());    // Surface size in compositor coords

        // Calc which outputs intersects the surface
        for(LOutput *o : compositor()->outputs())
        {
            if(o->rectC().intersects(currentRect))
                s->sendOutputEnterEvent(o);
            else
                s->sendOutputLeaveEvent(o);
        }

        // Draws the surface
        p->drawTextureC(
            s->texture(),        // Surface texture
            LRect(0,s->sizeB()), // The entire texture size
            currentRect);        // The destination rect on screen

        // Notifies the client that it can render the next frame 
        s->requestNextFrame();
    }

    // Manualy draw the cursor if hardware composition is not supported
    if(!cursor()->hasHardwareSupport(this))
    {
        p->drawTextureC(
            cursor()->texture(),                   
            LRect(0,cursor()->texture()->sizeB()),
            cursor()->rectC());
    }

    // Check if there is a drag & drop session going on with icon
    if(seat()->dndManager()->icon())
    {
        LSurface *s = seat()->dndManager()->icon()->surface();

        if(!s->mapped())
            return;

        // Sets the position of the icon to be the same as the cursor's position
        s->setPosC(cursor()->posC());

        p->drawTextureC(
                    s->texture(),
                    LRect(0, s->sizeB()),
                    LRect(s->rolePosC(),s->sizeC()));

        s->requestNextFrame();
    }
}
//! [paintGL]

//! [resizeGL]
void LOutput::resizeGL()
{
    repaint();
}
//! [resizeGL]

//! [uninitializeGL]
void LOutput::uninitializeGL()
{
    /* No default implementation */
}
//! [uninitializeGL]
