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
#include <LLayerRole.h>

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
    const bool sessionLocked { sessionLockManager()->state() != LSessionLockManager::Unlocked };
    const LRect availGeo { pos() + availableGeometry().pos(), availableGeometry().size() };
    LPainter::TextureParams params;
    LRect currentRect;
    LPainter *p { painter() };
    p->clearScreen();

    if (seat()->dnd()->icon())
        seat()->dnd()->icon()->surface()->raise();

    static const auto surfaceShouldBeSkipped = [this, sessionLocked](LSurface *s) -> bool
    {
        // An unmapped surface usually indicates it doesn't have a buffer
        if (!s->mapped())
            return true;

        // Should be displayed, for example, as a thumbnail in a panel instead
        if (s->minimized())
            return true;

        // Use LCursor instead for rendering the cursor
        if (s->cursorRole())
        {
            // But let the client update it
            s->requestNextFrame();
            return true;
        }

        // Layer or screen lock surfaces should be displayed only on their exclusive output or not at all
        if ((s->layerRole() && s->layerRole()->exclusiveOutput() != this) ||
            (s->sessionLock() && s->sessionLock()->exclusiveOutput() != this))
            return true;

        // Other roles with a different exclusive output, e.g. maximized or fullscreen toplevels
        if (s->role() && s->role()->exclusiveOutput() && s->role()->exclusiveOutput() != this)
            return true;

        // If the session is locked only surfaces from the locking client should be displayed
        if (sessionLocked && s->client() != sessionLockManager()->client())
            return true;

        return false;
    };

    static const auto surfaceShouldBeClippedToAvailableGeometry = [](LSurface *s) -> bool
    {
        // Popups are usually constrained to the entire output rect instead
        if (s->popup())
            return false;

        // Fullscreen toplevels should not be clipped
        if (s->toplevel() && !s->toplevel()->fullscreen())
            return true;

        // Toplevels or subsurfaces child of a non fullscreen toplevel
        if (s->subsurface() || s->toplevel())
        {
            const LSurface *topmostParent { s->topmostParent() };

            if (topmostParent && topmostParent->toplevel() && !topmostParent->toplevel()->fullscreen())
                return true;
        }

        return false;
    };

    // Draw every surface
    for (LSurface *s : compositor()->surfaces())
    {
        if (surfaceShouldBeSkipped(s))
            continue;

        // Current surface rect
        currentRect.setPos(s->rolePos());
        currentRect.setSize(s->size());

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

        // Clip to available geometry
        if (surfaceShouldBeClippedToAvailableGeometry(s))
            currentRect.clip(availGeo);

        // Draw the surface
        p->drawRect(currentRect);

        // Notify the client it can now render a new surface frame
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
