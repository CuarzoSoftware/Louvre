#include <LTextureView.h>
#include <LAnimation.h>
#include <LTimer.h>
#include <LSeat.h>

#include "Workspace.h"
#include "Compositor.h"
#include "Surface.h"
#include "LCursor.h"
#include "Output.h"
#include "Toplevel.h"
#include "ToplevelView.h"
#include "Global.h"
#include "Dock.h"
#include "Client.h"
#include "App.h"
#include "Topbar.h"
#include "src/Tooltip.h"

Surface::Surface(const void *params) :
    LSurface(params),
    view(this, &G::compositor()->surfacesLayer),
    minimizeAnim(400 * DEBUG_ANIM_SPEED)
{
    view.setVisible(false);

    firstMapAnim.setOnUpdateCallback([this](LAnimation *anim){

        const Float32 ease { 1.f - powf(1.f - anim->value(), 6.f) };
        view.setOpacity(1.f);
        getView()->setOpacity(ease);

        Surface *next = (Surface*)nextSurface();

        while (next)
        {
            if (next->isSubchildOf(this) && !next->minimized())
            {
                next->view.setOpacity(1.f);
                next->getView()->setOpacity(ease);
            }

            next = (Surface*)next->nextSurface();
        }

        repaintOutputs();
    });

    firstMapAnim.setOnFinishCallback([this](LAnimation *){
        getView()->setOpacity(1.f);
        view.setOpacity(1.f);

        Surface *next = (Surface*)nextSurface();

        while (next)
        {
            if (next->isSubchildOf(this) && !next->minimized())
            {
                next->view.setOpacity(1.f);
                next->getView()->setOpacity(1.f);
            }

            next = (Surface*)next->nextSurface();
        }
        repaintOutputs();

        if (tl() && tl()->requestedFullscreenOnFirstMap)
        {
            tl()->requestedFullscreenOnFirstMap = false;
            tl()->setFullscreenRequest(nullptr);
        }
    });
}

Surface::~Surface()
{
    firstMapTimer.cancel();

    if (toplevel())
        tl()->decoratedView.reset();

    while (!minimizedViews.empty())
        delete minimizedViews.back();

    if (thumbnailFullsizeView)
        delete thumbnailFullsizeView;

    if (thumbnailFullSizeTex)
        delete thumbnailFullSizeTex;

    if (thumbnailTex)
        delete thumbnailTex;
}

Surface *Surface::searchSessionLockParent(Surface *parent)
{
    if (parent)
    {
        if (parent->sessionLock())
            return parent;

        return searchSessionLockParent(static_cast<Surface*>(parent->parent()));
    }

    return nullptr;
}

LView *Surface::getView() const
{
    if (tl() && tl()->decoratedView)
        return tl()->decoratedView.get();

    return (LView*)&view;
}

void Surface::parentChanged()
{
    if (cursorRole())
    {
        getView()->setVisible(false);
        return;
    }

    if (parent())
    {
        Surface *sessionLockParent { searchSessionLockParent(static_cast<Surface*>(parent())) };

        if (sessionLockParent)
        {
            getView()->setParent(sessionLockParent->getView()->parent());
            return;
        }

        class Toplevel *tl = G::searchFullscreenParent((Surface*)parent());

        if (tl)
        {
            getView()->setParent(((Surface*)tl->surface())->getView()->parent());
            getView()->enableParentOffset(true);

            for (Workspace *ws : tl->fullscreenOutput->workspaces)
                ws->clipChildren();
        }
    }
}

void Surface::mappingChanged()
{
    if (cursorRole())
    {
        view.setVisible(false);
        return;
    }

    if (mapped())
    {
        compositor()->repaintAllOutputs();

        if (!firstMap)
        {
            view.setVisible(true);
            getView()->setVisible(true);
            getView()->setParent(&G::compositor()->surfacesLayer);
            raise();
            return;
        }

        Client *client { static_cast<Client*>(this->client()) };

        if (client->app)
        {
            // Stop dock App icon animation
            if (client->app->state != App::Running)
                client->app->state = App::Running;
        }
        else
        {   // If no App then is a non pinned dock App
            client->createNonPinnedApp();
            seat()->keyboard()->focusChanged();
        }

        if (toplevel())
        {
            firstMapTimer.start(10);
            requestNextFrame(false);

            LSurface *next { this };

            while (next)
            {
                if (next->isSubchildOf(this))
                    next->requestNextFrame(false);

                next = next->nextSurface();
            }
        }

        if (dndIcon())
            setPos(cursor()->pos());

        firstMap = false;
        requestNextFrame(false);

        Surface *parent { static_cast<Surface*>(this->parent()) };

        if ((!dndIcon() && !toplevel() && !subsurface()) || (subsurface() && parent && parent->view.visible()))
            getView()->setVisible(true);
    }
    else
    {
        if (hasPointerFocus())
            seat()->pointer()->setFocus(nullptr);

        if (tl())
        {
            if (!tl()->fullscreen())
                G::compositor()->fadeOutSurface(role(), 250);
        }
        else if (popup())
        {
            G::compositor()->fadeOutSurface(role(), 90);
        }

        view.repaint();
    }
}

void Surface::orderChanged()
{
    if (toplevel() && toplevel()->fullscreen())
        return;

    Surface *prevSurface { static_cast<Surface*>(this->prevSurface()) };
    LView *view { getView() };

    while (prevSurface != nullptr)
    {
        if (prevSurface->getView()->parent() == view->parent())
            break;

        prevSurface = static_cast<Surface*>(prevSurface->prevSurface());
    }

    if (prevSurface)
    {
        if (prevSurface->getView()->parent() == getView()->parent())
            view->insertAfter(prevSurface->getView());
    }
    else
        view->insertAfter(nullptr);
}

void Surface::roleChanged()
{
    if (roleId() == LSurface::Cursor)
    {
        view.setVisible(false);
        view.setParent(nullptr);
    }
    else if (roleId() == LSurface::DNDIcon)
    {
        setPos(cursor()->pos());
        getView()->setParent(&G::compositor()->overlayLayer);
        getView()->enableClipping(false);
        getView()->setVisible(true);
        getView()->enableParentOffset(false);
        raise();
    }
    else if (roleId() == LSurface::SessionLock || roleId() == LSurface::Popup)
    {
        getView()->setParent(&G::compositor()->overlayLayer);
    }
}

void Surface::bufferSizeChanged()
{
    view.repaint();
}

void Surface::minimizedChanged()
{
    if (minimized() && cursor()->output())
    {
        // When a surface is minimized, its children are too, so lets just hide them
        if (!toplevel())
        {
            view.setVisible(false);
            return;
        }

        minimizedOutput = (Output*)cursor()->output();

        // Render the surface, all its decorations and subsurfaces into a texture
        thumbnailFullSizeTex = renderThumbnail(&minimizedTransRegion);

        // Create a smaller scaled version for the dock
        Float32 s {Float32(DOCK_ITEM_HEIGHT) };
        thumbnailTex = thumbnailFullSizeTex->copy(LSize((s * thumbnailFullSizeTex->sizeB().w()) /thumbnailFullSizeTex->sizeB().h(), s) * 3.5f);

        // Create a view for thumbnailFullSizeTex (we only need one)
        thumbnailFullsizeView = new LTextureView(thumbnailFullSizeTex, getView()->parent());
        thumbnailFullsizeView->setBufferScale(2);
        thumbnailFullsizeView->enableParentOpacity(false);
        thumbnailFullsizeView->setPos(rolePos());
        thumbnailFullsizeView->setTranslucentRegion(&minimizedTransRegion);
        thumbnailFullsizeView->enableDstSize(true);
        thumbnailFullsizeView->setDstSize(thumbnailFullsizeView->texture()->sizeB() / thumbnailFullsizeView->bufferScale());

        // Hide the surface as we will show thumbnailFullsizeView instead
        getView()->setVisible(false);

        // We will move the fullsize view to the dock where the cursor is currently at
        DockItem *dstDockItem { nullptr };

        // Create a dock item for each output dock
        for (Output *o : G::outputs())
        {
            DockItem *minView { new DockItem(this, &o->dock) };

            if (cursor()->output() == o)
                dstDockItem = minView;
        }

        minimizeAnim.setOnUpdateCallback(
            [this, dstDockItem](LAnimation *anim)
            {
                const Float32 x { 1.f - powf(1.f - anim->value(), 3.f) };

                // Animate all docks items
                for (DockItem *item : minimizedViews)
                {
                    item->setScalingVector(x);

                    if (!item->dock->dockContainer.parent())
                        item->dock->dockContainer.setParent(item->dock);

                    item->dock->update();
                }

                // Scale and move fullsize view to the dock
                LRegion trans { minimizedTransRegion };
                trans.multiply(1.f - x);
                thumbnailFullsizeView->setTranslucentRegion(&trans);
                thumbnailFullsizeView->setDstSize((thumbnailFullsizeView->texture()->sizeB() / thumbnailFullsizeView->bufferScale()) * (1.f - x));
                thumbnailFullsizeView->setPos((dstDockItem->pos() + dstDockItem->size()) * x +
                         minimizeStartRect.pos() * (1.f - x));
            });

        minimizeAnim.setOnFinishCallback([this](LAnimation *)
        {
            // Finish docks items animations
            for (DockItem *item : minimizedViews)
            {
                item->setScalingVector(1.f);
                item->enableScaling(false);
                item->dock->update();

                if (item->dock->visiblePercent == 0.f)
                    item->dock->dockContainer.setParent(nullptr);
            }

            // Hide the resized fullsize view
            thumbnailFullsizeView->setVisible(false);
        });

        minimizeAnim.start();

        if (toplevel())
            toplevel()->configureState(toplevel()->pendingConfiguration().state &~LToplevelRole::Activated);
    }
    else
    {
        if (minimizedOutput)
        {
            minimizedOutput->setWorkspace(minimizedOutput->workspaces.front(), 600, 4.f);
            minimizedOutput = nullptr;
        }

        // Destroy minimized views
        while (!minimizedViews.empty())
        {
            Dock *dock { minimizedViews.back()->dock };
            delete minimizedViews.back();
            dock->update();
        }

        // Destroy the resized fullsize view
        if (thumbnailFullsizeView)
        {
            delete thumbnailFullsizeView;
            thumbnailFullsizeView = nullptr;

            // Destroy textures
            delete thumbnailFullSizeTex;
            thumbnailFullSizeTex = nullptr;
            delete thumbnailTex;
            thumbnailTex = nullptr;
        }

        getView()->setVisible(true);
        getView()->enablePointerEvents(true);
        raise();
    }
}

LTexture *Surface::renderThumbnail(LRegion *transRegion)
{
    LBox box { getView()->boundingBox() };

    minimizeStartRect = LRect(box.x1, box.y1, box.x2 - box.x1, box.y2 - box.y1);

    LSceneView tmpView(minimizeStartRect.size() * 2, 2);
    tmpView.setPos(minimizeStartRect.pos());

    LView *prevParent { getView()->parent() };
    getView()->setParent(&tmpView);

    struct TMPList
    {
        LSurfaceView *view;
        LView *parent;
    };

    std::list<TMPList>tmpChildren;

    Surface *next { this };
    while ((next = (Surface*)next->nextSurface()))
    {
        if (next->parent() == this && next->subsurface())
        {
            tmpChildren.emplace_back(&next->view, next->view.parent());
            next->view.enableParentOffset(false);
            next->view.setParent(&tmpView);
        }
    }

    getView()->enableParentOffset(false);

    tmpView.render();

    if (transRegion)
    {
        *transRegion = *tmpView.translucentRegion();
        transRegion->offset(LPoint() - tmpView.pos());
    }

    LTexture *renderedThumbnail { tmpView.texture()->copy() };
    getView()->enableParentOffset(true);
    getView()->setParent(prevParent);

    while (!tmpChildren.empty())
    {
        tmpChildren.front().view->enableParentOffset(true);
        tmpChildren.front().view->setParent(tmpChildren.front().parent);
        tmpChildren.pop_front();
    }

    return renderedThumbnail;
}

void Surface::unminimize(DockItem *clickedItem)
{
    // Show the resized fullsize view
    thumbnailFullsizeView->setVisible(true);
    thumbnailFullsizeView->insertAfter(getView()->parent()->children().back());

    // Setup dock items
    for (DockItem *item : minimizedViews)
    {
        item->enablePointerEvents(false);
        item->setOpacity(1.f);
        item->setScalingVector(1.f);
        item->enableScaling(true);
    }

    if (toplevel())
    {
        toplevel()->configureState(toplevel()->pendingConfiguration().state | LToplevelRole::Activated);
        requestNextFrame(false);
    }

    minimizeAnim.setOnUpdateCallback(
    [this, clickedItem](LAnimation *anim)
    {
        const Float32 x { 1.f - powf(1.f - anim->value(), 3.f) };

        // Animate all docks items
        for (DockItem *item : minimizedViews)
        {
            if (!item->dock->dockContainer.parent())
                item->dock->dockContainer.setParent(item->dock);

            item->setScalingVector(1.f - x);
            item->dock->update();
        }

        LRegion trans { minimizedTransRegion };
        trans.multiply(x);
        thumbnailFullsizeView->setTranslucentRegion(&trans);
        thumbnailFullsizeView->setDstSize((thumbnailFullsizeView->texture()->sizeB() / thumbnailFullsizeView->bufferScale()) * x);

        // Scale and move fullsize view to the dock
        thumbnailFullsizeView->setPos((clickedItem->pos() + clickedItem->size()) * (1.f - x) +
                 minimizeStartRect.pos() * x);
    });

    minimizeAnim.setOnFinishCallback(
    [this](LAnimation *)
    {
        for (DockItem *item : minimizedViews)
            if (item->dock->visiblePercent == 0.f)
                item->dock->dockContainer.setParent(nullptr);

        setMinimized(false);

        if (G::tooltip())
            G::tooltip()->hide();
    });

    minimizeAnim.start();
}

void Surface::damageChanged()
{
    repaintOutputs();
}

void Surface::preferVSyncChanged()
{
    if (tl() && tl()->fullscreenOutput && tl()->fullscreenOutput->currentWorkspace == tl()->fullscreenWorkspace)
    {
        tl()->fullscreenOutput->enableVSync(preferVSync());
        tl()->fullscreenOutput->topbar.update();
    }
}

void Surface::onToplevelFirstMap() noexcept
{
    if (!cursor()->output() ||!toplevel() || !mapped() || minimized())
        return;

    LOutput *output { cursor()->output() };
    const LPoint outputPosG { output->pos() + output->availableGeometry().pos() };
    LSize tlSize { toplevel()->windowGeometry().size() };

    if (tl()->supportServerSideDecorations())
        tlSize += LSize(0, tl()->extraGeometry().top);

    setPos(outputPosG + (output->availableGeometry().size() - tlSize)/2);

    if (pos().x() < outputPosG.x())
        setX(outputPosG.x());

    if (pos().y() < outputPosG.y())
        setY(outputPosG.y());

    if (!tl()->pendingConfiguration().state.check(LToplevelRole::Fullscreen) && tl()->supportServerSideDecorations() &&
        output->availableGeometry().size().h() <= tlSize.h())
    {
        tl()->setMaximizedRequest();
    }

    Surface *next { static_cast<Surface*>(nextSurface()) };
    view.setVisible(true);
    getView()->setVisible(true);

    while (next)
    {
        if (next->isSubchildOf(this) && !next->minimized())
        {
            view.setVisible(true);
            next->getView()->setVisible(true);
        }

        next = static_cast<Surface*>(next->nextSurface());
    }

    output->repaint();

    firstMapAnim.setDuration(400 *DEBUG_ANIM_SPEED);
    firstMapAnim.start();
}
