#include <LTextureView.h>
#include <LAnimation.h>

#include "LTimer.h"
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

Surface::Surface(LSurface::Params *params) : LSurface(params)
{
    view = new LSurfaceView(this, &G::compositor()->surfacesLayer);
    view->setVisible(false);
}

Surface::~Surface()
{
    if (firstMapTimer)
        firstMapTimer->cancel();

    if (toplevel())
    {
        if (tl()->decoratedView)
        {
            delete tl()->decoratedView;
            tl()->decoratedView = nullptr;
        }
    }

    if (minimizeAnim)
        minimizeAnim->stop();

    while (!minimizedViews.empty())
        delete minimizedViews.back();

    if (thumbnailFullsizeView)
        delete thumbnailFullsizeView;

    if (thumbnailFullSizeTex)
        delete thumbnailFullSizeTex;

    if (thumbnailTex)
        delete thumbnailTex;

    delete view;
}

LView *Surface::getView() const
{
    if (tl() && tl()->decoratedView)
        return tl()->decoratedView;

    return view;
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
        class Toplevel *tl = G::searchFullscreenParent((Surface*)parent());

        if (tl)
        {
            getView()->setParent(((Surface*)tl->surface())->getView()->parent());
            getView()->enableParentOffset(true);

            for (Workspace *ws : tl->fullscreenOutput->workspaces)
                ws->clipChildren();
        }

        parent()->raise();
    }
}

void Surface::mappingChanged()
{
    if (cursorRole())
        view->setVisible(false);

    if (mapped())
    {
        if (firstMap)
        {
            // Stop dock App icon animation
            Client *cli = (Client*)client();

            if (cli->app)
            {
                if (cli->app->state != App::Running)
                    cli->app->state = App::Running;
            }
            // If no App then is a non pinned dock App
            else
            {
                cli->createNonPinnedApp();
                seat()->keyboard()->focusChanged();
            }

            if (toplevel())
            {
                if (!firstMapTimer)
                {
                    firstMapTimer = new LTimer([this](LTimer*)
                    {
                        firstMapTimer = nullptr;

                        if (!toplevel() || !mapped() || minimized())
                            return;

                        LPoint outputPosG = compositor()->cursor()->output()->pos() + LPoint(0, TOPBAR_HEIGHT);
                        LSize outputSizeG = compositor()->cursor()->output()->size() - LSize(0, TOPBAR_HEIGHT);

                        setPos(outputPosG + (outputSizeG - toplevel()->windowGeometry().size())/2);

                        if (pos().x() < outputPosG.x())
                            setX(outputPosG.x());

                        if (pos().y() < TOPBAR_HEIGHT)
                            setY(TOPBAR_HEIGHT);

                        Surface *next = (Surface*)nextSurface();

                        view->setVisible(true);
                        getView()->setVisible(true);

                        while (next)
                        {
                            if (next->isSubchildOf(this) && !next->minimized())
                            {
                                view->setVisible(true);
                                next->getView()->setVisible(true);
                            }

                            next = (Surface*)next->nextSurface();
                        }

                        compositor()->cursor()->output()->repaint();
                    });

                    firstMapTimer->start(200, true);
                }

                toplevel()->configure(LToplevelRole::Activated);

                requestNextFrame(false);

                LSurface *next = this;

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

            Surface *par = (Surface*)parent();

            if ((!dndIcon() && !toplevel() && !subsurface()) || (subsurface() && par && par->view->visible()))
                getView()->setVisible(true);
        }

        compositor()->repaintAllOutputs();
    }
    else
    {
        if (seat()->pointer()->focus() == this)
            seat()->pointer()->setFocus(nullptr);

        if (toplevel() && toplevel()->fullscreen())
            toplevel()->configure(toplevel()->states() &~LToplevelRole::Fullscreen);

        view->repaint();
    }
}

void Surface::orderChanged()
{
    Surface *prev = (Surface*)prevSurface();

    LView *v = getView();

    while (prev != nullptr)
    {
        if (prev->getView()->parent() == v->parent())
            break;

        prev = (Surface*)prev->prevSurface();
    }

    if (prev)
        v->insertAfter(prev->getView(), false);
    else
        v->insertAfter(nullptr, false);
}

void Surface::roleChanged()
{
    if (roleId() == LSurface::Cursor)
    {
        view->setVisible(false);
        view->setParent(nullptr);
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
    else if (roleId() == LSurface::Toplevel || roleId() == LSurface::Popup)
    {
        sendOutputEnterEvent(cursor()->output());
    }
}

void Surface::bufferSizeChanged()
{
    view->repaint();
}

void Surface::minimizedChanged()
{
    if (minimized())
    {
        // When a surface is minimized, its children are too, so lets just hide them
        if (!toplevel())
        {
            view->setVisible(false);
            return;
        }

        minimizedOutput = (Output*)cursor()->output();

        // Render the surface, all its decorations and subsurfaces into a texture
        thumbnailFullSizeTex = renderThumbnail(&minimizedTransRegion);

        // Create a smaller scaled version for the dock
        Float32 s = float(DOCK_ITEM_HEIGHT);
        thumbnailTex = thumbnailFullSizeTex->copyB(LSize((s * thumbnailFullSizeTex->sizeB().w()) /thumbnailFullSizeTex->sizeB().h(), s) * 3.5f);

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
        DockItem *dstDockItem = nullptr;

        // Create a dock item for each output dock
        for (Output *o : G::outputs())
        {
            DockItem *minView = new DockItem(this, o->dock);

            if (cursor()->output() == o)
                dstDockItem = minView;
        }

        minimizeAnim = LAnimation::create(300,
        [this, dstDockItem](LAnimation *anim)
        {
            // Transform linear curve to ease out
            Float32 easeOut = 1.f - powf(1.f - anim->value(), 2.f);

            // Animate all docks items
            for (DockItem *item : minimizedViews)
            {
                item->setScalingVector(easeOut);
                item->dock->update();
            }

            // Scale and move fullsize view to the dock
            LRegion trans = minimizedTransRegion;
            trans.multiply((1.f - easeOut));
            thumbnailFullsizeView->setTranslucentRegion(&trans);
            thumbnailFullsizeView->setDstSize((thumbnailFullsizeView->texture()->sizeB() / thumbnailFullsizeView->bufferScale()) * (1.f - easeOut));
            thumbnailFullsizeView->setPos((dstDockItem->pos() + dstDockItem->size()) * easeOut +
                     minimizeStartRect.pos() * (1.f - easeOut));
        },
        [this](LAnimation *)
        {
            // Finish docks items animations
            for (DockItem *item : minimizedViews)
            {
                item->setScalingVector(1.f);
                item->enableScaling(false);
                item->dock->update();
            }

            // Hide the resized fullsize view
            thumbnailFullsizeView->setVisible(false);
            minimizeAnim = nullptr;
        });

        minimizeAnim->start();

        if (toplevel())
            toplevel()->configure(toplevel()->states() &~LToplevelRole::Activated);
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
            Dock *dock = minimizedViews.back()->dock;
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

            minimizeAnim = nullptr;
        }

        raise();

        if (toplevel())
            toplevel()->configure(toplevel()->states() | LToplevelRole::Activated);

        getView()->setVisible(true);
        getView()->enableInput(true);
    }
}

LTexture *Surface::renderThumbnail(LRegion *transRegion)
{
    LBox box = getView()->boundingBox();

    minimizeStartRect = LRect(box.x1, box.y1, box.x2 - box.x1, box.y2 - box.y1);

    LSceneView tmpView(minimizeStartRect.size() * 2, 2);
    tmpView.setPos(minimizeStartRect.pos());

    LView *prevParent = getView()->parent();
    getView()->setParent(&tmpView);

    struct TMPList
    {
        LSurfaceView *view;
        LView *parent;
    };

    std::list<TMPList>tmpChildren;

    Surface *next = this;
    while ((next = (Surface*)next->nextSurface()))
    {
        if (next->parent() == this && next->subsurface())
        {
            tmpChildren.push_back({next->view, next->view->parent()});
            next->view->enableParentOffset(false);
            next->view->setParent(&tmpView);
        }
    }

    for (LView *child : tmpView.children())
        G::setBlendFuncWithChildren(child, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    getView()->enableParentOffset(false);

    tmpView.render();

    if (transRegion)
    {
        *transRegion = *tmpView.translucentRegion();
        transRegion->offset(LPoint() - tmpView.pos());
    }

    for (LView *child : tmpView.children())
        G::setBlendFuncWithChildren(child, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    LTexture *renderedThumbnail = tmpView.texture()->copyB();
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
        item->enableInput(false);
        item->setOpacity(1.f);
        item->setScalingVector(1.f);
        item->enableScaling(true);
    }

    minimizeAnim = LAnimation::create(300,
    [this, clickedItem](LAnimation *anim)
    {
        // Transform linear curve to ease out
        Float32 exp = powf(anim->value(), 2.f);

        // Animate all docks items
        for (DockItem *item : minimizedViews)
        {
            item->setScalingVector(1.f - exp);
            item->dock->update();
        }

        LRegion trans = minimizedTransRegion;
        trans.multiply(exp);
        thumbnailFullsizeView->setTranslucentRegion(&trans);
        thumbnailFullsizeView->setDstSize((thumbnailFullsizeView->texture()->sizeB() / thumbnailFullsizeView->bufferScale()) * exp);

        // Scale and move fullsize view to the dock
        thumbnailFullsizeView->setPos((clickedItem->pos() + clickedItem->size()) * (1.f - exp) +
                 minimizeStartRect.pos() * exp);
    },
    [this](LAnimation *)
    {
        setMinimized(false);
        minimizeAnim = nullptr;
    });

    minimizeAnim->start();
}

void Surface::damageChanged()
{
    repaintOutputs();
}
