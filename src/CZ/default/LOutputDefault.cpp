#include <CZ/Louvre/LLog.h>
#include <CZ/Louvre/LOutput.h>
#include <CZ/Louvre/LPainter.h>
#include <CZ/Louvre/LCompositor.h>
#include <CZ/Louvre/LSurface.h>
#include <CZ/Louvre/LSeat.h>
#include <CZ/Louvre/LDND.h>
#include <CZ/Louvre/Roles/LDNDIconRole.h>
#include <CZ/Louvre/LCursor.h>
#include <CZ/Louvre/LPointer.h>
#include <CZ/Louvre/LSessionLockManager.h>
#include <CZ/Louvre/Roles/LSessionLockRole.h>
#include <CZ/Louvre/Roles/LLayerRole.h>

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
    const SkIRect availGeo { SkIRect::MakePtSize(pos() + availableGeometry().topLeft(), availableGeometry().size()) };
    LPainter::TextureParams params;
    SkIRect currentRect { 0, 0, 0, 0 };
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
        currentRect = SkIRect::MakePtSize(s->rolePos(), s->size());

        // Calc which outputs intersect the surface
        for (LOutput *o : compositor()->outputs())
        {
            if (SkIRect::Intersects(o->rect(), currentRect))
                s->sendOutputEnterEvent(o);
            else
                s->sendOutputLeaveEvent(o);
        }

        params.srcRect = s->srcRect();
        params.dstSize = s->size();
        params.srcScale = s->bufferScale();
        params.srcTransform = s->bufferTransform();
        params.texture = s->texture();
        params.pos = currentRect.topLeft();

        // Bind the surface texture
        p->bindTextureMode(params);

        // Clip to available geometry
        if (surfaceShouldBeClippedToAvailableGeometry(s))
            // If the intersected area is zero
            if (!currentRect.intersect(availGeo))
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

//! [leaseRequest]
bool LOutput::leaseRequest(LClient *client)
{
    L_UNUSED(client)
    return true;
}
//! [leaseRequest]

//! [leaseChanged]
void LOutput::leaseChanged()
{
    LLog::debug("[%s] Leased by client: %s.", name(), lease() ? "YES" : "NO");
}
//! [leaseChanged]

//! [availableGeometryChanged]
void LOutput::availableGeometryChanged()
{
    const SkIRect availGeo { SkIRect::MakePtSize(pos() + availableGeometry().topLeft(), availableGeometry().size()) };

    for (LSurface *surface : compositor()->surfaces())
    {        
        LToplevelRole *tl { surface->toplevel() };

        if (tl && !tl->fullscreen())
        {
            const SkISize toplevelExtraSize { tl->extraGeometry().left + tl->extraGeometry().right, tl->extraGeometry().top + tl->extraGeometry().bottom };
            SkIRect toplevelRect {
                SkIRect::MakePtSize(
                    surface->pos(),
                    SkISize(
                        tl->windowGeometry().width() + toplevelExtraSize.width(),
                        tl->windowGeometry().height() + toplevelExtraSize.height()))
            };

            if (compositor()->mostIntersectedOutput(toplevelRect) != this)
                continue;

            if (tl->maximized())
            {
                surface->setPos(availGeo.topLeft());
                tl->configureSize(
                    SkISize(
                        availGeo.width() - toplevelExtraSize.width(),
                        availGeo.height() - toplevelExtraSize.height()));
            }
            else
            {
                if (exclusiveEdges().right != 0 && toplevelRect.x() + toplevelRect.width() > availGeo.x() + availGeo.width())
                    toplevelRect.offsetTo(
                        availGeo.x() + availGeo.width() - toplevelRect.width(),
                        toplevelRect.y());

                if (exclusiveEdges().bottom != 0 && toplevelRect.y() + toplevelRect.height() > availGeo.y() + availGeo.height())
                    toplevelRect.offsetTo(
                        toplevelRect.x(),
                        availGeo.y() + availGeo.height() - toplevelRect.height());

                if (exclusiveEdges().left != 0 && toplevelRect.x() < availGeo.x())
                    toplevelRect.offsetTo(availGeo.x(), toplevelRect.y());

                if (exclusiveEdges().top != 0 && toplevelRect.y() < availGeo.y())
                    toplevelRect.offsetTo(toplevelRect.x(), availGeo.y());

                bool needsConfigure { false };

                if (exclusiveEdges().right != 0 && toplevelRect.width() > availGeo.width())
                {
                    toplevelRect.fRight = toplevelRect.fLeft + availGeo.width();
                    needsConfigure = true;
                }

                if (exclusiveEdges().bottom != 0 && toplevelRect.height() > availGeo.height())
                {
                    toplevelRect.fBottom = toplevelRect.fTop + availGeo.height();
                    needsConfigure = true;
                }

                if (needsConfigure)
                    tl->configureSize(
                        SkISize(
                            toplevelRect.width() - toplevelExtraSize.width(),
                            toplevelRect.height() - toplevelExtraSize.height()));

                surface->setPos(toplevelRect.topLeft());
            }
        }
    }
}
//! [availableGeometryChanged]

//! [repaintFilter]
bool LOutput::repaintFilter()
{
    return true;
}
//! [repaintFilter]
