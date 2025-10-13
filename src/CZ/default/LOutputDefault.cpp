#include "LClient.h"
#include "Protocols/Wayland/GOutput.h"
#include <CZ/Core/Events/CZPresentationEvent.h>

#include <CZ/Louvre/LLog.h>
#include <CZ/Louvre/Seat/LOutput.h>
#include <CZ/Louvre/LCompositor.h>
#include <CZ/Louvre/Roles/LSurface.h>
#include <CZ/Louvre/Seat/LSeat.h>
#include <CZ/Louvre/Seat/LDND.h>
#include <CZ/Louvre/Cursor/LCursor.h>
#include <CZ/Louvre/Seat/LPointer.h>
#include <CZ/Louvre/Manager/LSessionLockManager.h>
#include <CZ/Louvre/Roles/LDNDIconRole.h>
#include <CZ/Louvre/Roles/LSessionLockRole.h>
#include <CZ/Louvre/Roles/LLayerRole.h>
#include <CZ/Louvre/Roles/LSubsurfaceRole.h>
#include <CZ/Louvre/Roles/LPopupRole.h>

#include <CZ/Louvre/Protocols/PresentationTime/RPresentationFeedback.h>
#include <CZ/Louvre/Private/LOutputPrivate.h>

#include <CZ/Ream/RSurface.h>
#include <CZ/Ream/RPass.h>

#include <CZ/Core/CZAnimation.h>

using namespace CZ;

//! [initializeGL]
void LOutput::initializeGL()
{
    repaint();
}
//! [initializeGL]

//! [paintGL]
static void DrawSurface(RPainter *p, LSurface *s) noexcept
{
    RDrawImageInfo info {};
    info.dst = SkIRect::MakePtSize(s->rolePos(), s->size());
    info.src = s->srcRect();
    info.image = s->image();
    info.srcScale = s->scale();
    info.srcTransform = s->bufferTransform();

    // Calc which outputs intersect the surface
    for (LOutput *o : compositor()->outputs())
    {
        if (SkIRect::Intersects(o->rect(), info.dst))
            s->sendOutputEnterEvent(o);
        else
            s->sendOutputLeaveEvent(o);
    }

    // If the surface has an exclusive output, prevent leaks it into this one
    if (s->role() && s->role()->exclusiveOutput())
    {
        const SkRegion clip { s->role()->exclusiveOutput()->rect() };
        p->drawImage(info, &clip);
    }
    else
        p->drawImage(info);

    s->requestNextFrame();
}

static void DrawSubsurfaces(RPainter *p, const std::vector<LSubsurfaceRole*> &subsurfaces) noexcept
{
    for (auto *sub : subsurfaces)
    {
        if (!sub->surface()->mapped())
            continue;

        DrawSubsurfaces(p, sub->surface()->subsurfacesBelow());
        DrawSurface(p, sub->surface());
        DrawSubsurfaces(p, sub->surface()->subsurfacesAbove());
    }
}

static void DrawTree(RPainter *p, LSurface *s) noexcept
{
    if (!s->mapped() || (s->toplevel() && s->toplevel()->isMinimized())) return;

    DrawSubsurfaces(p, s->subsurfacesBelow());
    DrawSurface(p, s);
    DrawSubsurfaces(p, s->subsurfacesAbove());

    if (auto *toplevel = s->toplevel())
    {
        for (auto *child : toplevel->childPopups())
            DrawTree(p, child->surface());

        for (auto *child :toplevel->childToplevels())
            DrawTree(p, child->surface());
    }
    else if (auto *popup = s->popup())
    {
        for (auto *child : popup->childPopups())
            DrawTree(p, child->surface());
    }
    else if (auto *layerRole = s->layerRole())
    {
        for (auto *child : layerRole->childPopups())
            DrawTree(p, child->surface());
    }

    // Session lock roles only contain subsurfaces
}

void LOutput::paintGL()
{
    // Create an RSurface for the current output image
    auto surface { RSurface::WrapImage(image()) };

    // Configure the geometry to match the output
    surface->setGeometry({
        .viewport = SkRect::Make(rect()),
        .dst = SkRect::Make(image()->size()),
        .transform = transform()});

    // Begin a pass with access to RPainter
    auto pass { surface->beginPass(RPassCap_Painter) };

    auto *p { pass->getPainter() };
    p->setColor(0xFF00356B);
    p->clear();

    const bool sessionIsLocked { sessionLockManager()->state() != LSessionLockManager::Unlocked };

    if (sessionIsLocked)
    {
        // Session lock surface assigned to this output
        if (sessionLockRole())
            DrawTree(p, sessionLockRole()->surface());

        return;
    }

    if (seat()->dnd()->icon())
        seat()->dnd()->icon()->surface()->raise();

    // From LLayerBackground to LLayerOverlay layers
    for (const auto &layer : compositor()->layers())
    {
        for (LSurface *s : layer)
        {
            // Child surfaces are rendered by DrawTree
            if (s->parent())
                continue;

            // Cursor surfaces are rendered by LCursor
            if (s->cursorRole())
            {
                s->requestNextFrame();
                continue;
            }

            DrawTree(p, s);
        }
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
void LOutput::setGammaRequest(LClient *client, std::shared_ptr<const RGammaLUT> gamma)
{
    L_UNUSED(client)

    /* Sets the client gamma table */
    setGammaLUT(gamma);
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
    LLog(CZInfo, CZLN, "[{}] Leased by client: {}.", name(), lease() != nullptr);
}
//! [leaseChanged]

//! [availableGeometryChanged]
void LOutput::availableGeometryChanged()
{
    const SkIRect availGeo { SkIRect::MakePtSize(pos() + availableGeometry().topLeft(), availableGeometry().size()) };

    for (LSurface *surface : compositor()->surfaces())
    {        
        LToplevelRole *tl { surface->toplevel() };

        if (tl && !tl->isFullscreen())
        {
            SkIRect toplevelRect {
                SkIRect::MakePtSize(
                    surface->pos(),
                    tl->windowGeometry().size())
            };

            if (compositor()->mostIntersectedOutput(toplevelRect) != this)
                continue;

            if (tl->isMaximized())
            {
                surface->setPos(availGeo.topLeft());
                tl->configureSize(availGeo.size());
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
                    tl->configureSize(toplevelRect.size());

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

void LOutput::presentationEvent(const CZPresentationEvent &e) noexcept
{
    for (auto it = imp()->waitingPresentationFeedback.begin(); it != imp()->waitingPresentationFeedback.end();)
    {
        if (!(*it))
        {
            it = imp()->waitingPresentationFeedback.erase(it);
            continue;
        }

        if ((*it)->paintEventId != e.info.paintEventId)
        {
            it++;
            continue;
        }

        if (e.discarded)
        {
            (*it)->discarded();
        }
        else
        {
            for (auto *outputGlobal : (*it)->client()->outputGlobals())
                if (outputGlobal->output() == this)
                    (*it)->syncOutput(outputGlobal);

            (*it)->presented(e.info.time.tv_sec >> 32,
                             e.info.time.tv_sec & 0xffffffff,
                             e.info.time.tv_nsec,
                             e.info.period,
                             e.info.seq >> 32,
                             e.info.seq & 0xffffffff,
                             e.info.flags);
        }

        it = imp()->waitingPresentationFeedback.erase(it);
    }
}

bool LOutput::event(const CZEvent &e) noexcept
{
    if (e.type() == CZEvent::Type::Presentation)
    {
        presentationEvent((const CZPresentationEvent&)e);
        return true;
    }

    return LFactoryObject::event(e);
}
