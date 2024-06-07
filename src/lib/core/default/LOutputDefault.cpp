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

    static const auto surfaceShouldBeSkipped = [](LSurface *s, LOutput *output, bool sessionLocked) -> bool
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
        if ((s->layerRole() && s->layerRole()->exclusiveOutput() != output) ||
            (s->sessionLock() && s->sessionLock()->exclusiveOutput() != output))
            return true;

        // Other roles with a different exclusive output, e.g. maximized or fullscreen toplevels
        if (s->role() && s->role()->exclusiveOutput() && s->role()->exclusiveOutput() != output)
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
        if (surfaceShouldBeSkipped(s, this, sessionLocked))
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
            // If the intersected area is zero
            if (currentRect.clip(availGeo))
                continue;

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

    /* Sets the client gamma table */
    setGamma(gamma);
}
//! [setGammaRequest]

//! [availableGeometryChanged()]
void LOutput::availableGeometryChanged()
{
    const LRect availGeo { pos() + availableGeometry().pos(), availableGeometry().size() };

    for (LSurface *surface : compositor()->surfaces())
    {        
        LToplevelRole *tl { surface->toplevel() };

        if (tl && !tl->fullscreen())
        {
            const LSize toplevelExtraSize { tl->extraGeometry().left + tl->extraGeometry().right, tl->extraGeometry().top + tl->extraGeometry().bottom };
            LRect toplevelRect { surface->pos(), tl->windowGeometry().size() + toplevelExtraSize };

            if (compositor()->mostIntersectedOutput(toplevelRect) != this)
                continue;

            if (tl->maximized())
            {
                surface->setPos(availGeo.pos());
                tl->configureSize(availGeo.size()
                    - toplevelExtraSize);
            }
            else
            {
                if (exclusiveEdges().right != 0 && toplevelRect.x() + toplevelRect.w() > availGeo.x() + availGeo.w())
                    toplevelRect.setX(availGeo.x() + availGeo.w() - toplevelRect.w());

                if (exclusiveEdges().bottom != 0 && toplevelRect.y() + toplevelRect.h() > availGeo.y() + availGeo.h())
                    toplevelRect.setY(availGeo.y() + availGeo.h() - toplevelRect.h());

                if (exclusiveEdges().left != 0 && toplevelRect.x() < availGeo.x())
                    toplevelRect.setX(availGeo.x());

                if (exclusiveEdges().top != 0 && toplevelRect.y() < availGeo.y())
                    toplevelRect.setY(availGeo.y());

                bool needsConfigure { false };

                if (exclusiveEdges().right != 0 && toplevelRect.w() > availGeo.w())
                {
                    toplevelRect.setW(availGeo.w());
                    needsConfigure = true;
                }

                if (exclusiveEdges().bottom != 0 && toplevelRect.h() > availGeo.h())
                {
                    toplevelRect.setH(availGeo.h());
                    needsConfigure = true;
                }

                if (needsConfigure)
                    tl->configureSize(toplevelRect.size() - toplevelExtraSize);

                surface->setPos(toplevelRect.pos());
            }
        }
    }
}
//! [availableGeometryChanged()]
