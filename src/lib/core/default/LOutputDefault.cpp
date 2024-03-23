#include <LOutput.h>
#include <LPainter.h>
#include <LCompositor.h>
#include <LSurface.h>
#include <LSeat.h>
#include <LDND.h>
#include <LDNDIconRole.h>
#include <LCursor.h>
#include <LPointer.h>
#include <LSessionLockManager.h>
#include <LSessionLockRole.h>

using namespace Louvre;

//! [initializeGL]
void LOutput::initializeGL()
{
    painter()->setClearColor(1.f, 1.f, 1.f, 1.f);
}
//! [initializeGL]

//! [paintGL]
void LOutput::paintGL()
{
    const bool sessionLocked { compositor()->sessionLockManager()->state() != LSessionLockManager::Unlocked };

    LPainter *p { painter() };
    LPainter::TextureParams params;
    p->clearScreen();

    if (seat()->dnd()->icon())
        seat()->dnd()->icon()->surface()->raise();

    if (sessionLockRole())
        sessionLockRole()->surface()->raise();

    // Draw every surface
    for (LSurface *s : compositor()->surfaces())
    {
        // Skip some surfaces
        if (!s->mapped() || s->minimized() || s->cursorRole())
        {
            s->requestNextFrame();
            continue;
        }

        // Check if the session is locked
        if (sessionLocked)
        {
            if (sessionLockRole())
            {
                if (sessionLockRole()->surface() != s && !s->isSubchildOf(sessionLockRole()->surface()))
                    continue;
            }
            else
                break;
        }

        // Current surface rect
        const LRect currentRect {
            s->rolePos(),  // Role pos in surface coords
            s->size()};    // Surface size in surface coords

        // Calc which outputs intersect the surface
        for (LOutput *o : compositor()->outputs())
        {
            if (o->rect().intersects(currentRect))
                s->sendOutputEnterEvent(o);
            else
                s->sendOutputLeaveEvent(o);
        }

        params.srcRect = s->srcRect();
        params.dstSize = s->size();
        params.srcScale = s->bufferScale();
        params.srcTransform = s->bufferTransform();
        params.texture = s->texture();
        params.pos = currentRect.pos();

        // Bind the surface texture
        p->bindTextureMode(params);

        // Draw the surface
        p->drawRect(currentRect);

        // Notify the client it can now render a new surface frame
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

//! [setGammaRequest]
void LOutput::setGammaRequest(LClient *client, const LGammaTable *gamma)
{
    L_UNUSED(client)
    L_UNUSED(gamma)

    /* No default implementation */
}
//! [setGammaRequest]
