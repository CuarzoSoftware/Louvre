#include <LTextureView.h>
#include <LAnimation.h>

#include "Compositor.h"
#include "LTime.h"
#include "Surface.h"
#include "LCursor.h"
#include "Output.h"
#include "Toplevel.h"
#include "ToplevelView.h"
#include "Global.h"
#include "Dock.h"

Surface::Surface(LSurface::Params *params) : LSurface(params)
{
    view = new LSurfaceView(this, G::compositor()->surfacesLayer);
}

Surface::~Surface()
{
    if (toplevel())
    {
        class Toplevel *tl = (class Toplevel*)toplevel();

        if (tl->decoratedView)
        {
            delete tl->decoratedView;
            tl->decoratedView = nullptr;
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
    class Toplevel *tl = (class Toplevel*)toplevel();

    if (tl && tl->decoratedView)
        return tl->decoratedView;

    return view;
}

static LView *searchFullscreenParent(Surface *parent)
{
    if (!parent)
        return nullptr;

    if (parent->toplevel() && parent->toplevel()->fullscreen())
    {
        Toplevel *tl = (Toplevel*)parent->toplevel();
        return tl->fullscreenOutput->fullscreenView;
    }

    return searchFullscreenParent((Surface*)parent->parent());
}
void Surface::parentChanged()
{
    if (parent())
    {
        LView *fullscreenView = searchFullscreenParent((Surface*)parent());

        if (fullscreenView)
        {
            getView()->setParent(fullscreenView);
            compositor()->raiseSurface(parent());
        }
    }
}

void Surface::mappingChanged()
{
    if (mapped())
    {
        if (firstMap)
        {
            firstMap = false;

            if (toplevel())
            {
                LPoint outputPosG = compositor()->cursor()->output()->pos() + LPoint(0, TOPBAR_HEIGHT);
                LSize outputSizeG = compositor()->cursor()->output()->size() - LSize(0, TOPBAR_HEIGHT);

                setPos(outputPosG + (outputSizeG - toplevel()->windowGeometry().size())/2);

                if (pos().x() < outputPosG.x())
                    setX(outputPosG.x());

                if (pos().y() < TOPBAR_HEIGHT)
                    setY(TOPBAR_HEIGHT);

                toplevel()->configure(LToplevelRole::Activated);
            }

            if (dndIcon())
                setPos(cursor()->pos());
        }

        compositor()->repaintAllOutputs();
    }
    else
    {
        if (seat()->pointer()->focusSurface() == this)
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

    if (prev)
        v->insertAfter(prev->getView(), false);
    else
        v->insertAfter(nullptr, false);
}

void Surface::roleChanged()
{
    if (roleId() == LSurface::Cursor)
        view->setVisible(false);
    if (roleId() == LSurface::DNDIcon)
        setPos(cursor()->pos());
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

        // Render the surface, all its decorations and subsurfaces into a texture
        thumbnailFullSizeTex = renderThumbnail();

        // Create a smaller scaled version for the dock
        thumbnailTex = thumbnailFullSizeTex->copyB(LSize((DOCK_ITEM_HEIGHT * thumbnailFullSizeTex->sizeB().w()) /thumbnailFullSizeTex->sizeB().h(), DOCK_ITEM_HEIGHT) * view->bufferScale());

        // Create a view for thumbnailFullSizeTex (we only need one)
        thumbnailFullsizeView = new LTextureView(thumbnailFullSizeTex, getView()->parent());
        thumbnailFullsizeView->setBufferScale(view->bufferScale());
        thumbnailFullsizeView->enableScaling(true);
        thumbnailFullsizeView->enableParentOpacity(false);
        thumbnailFullsizeView->setPos(rolePos());

        // Hide the surface as we will show thumbnailFullsizeView instead
        getView()->setVisible(false);

        // We will move the fullsize view to the dock where the cursor is currently at
        DockItem *dstDockItem;

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
            thumbnailFullsizeView->setScalingVector(1.f - easeOut);
            thumbnailFullsizeView->setPos((dstDockItem->pos() + dstDockItem->size()) * easeOut +
                     minimizeStartRect.pos() * (1.f - easeOut));

            return true;
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

        compositor()->raiseSurface(this);

        if (toplevel())
            toplevel()->configure(toplevel()->states() | LToplevelRole::Activated);

        getView()->setVisible(true);
        getView()->enableInput(true);
    }
}

LTexture *Surface::renderThumbnail()
{
    LBox box = getView()->boundingBox();

    minimizeStartRect = LRect(box.x1, box.y1, box.x2 - box.x1, box.y2 - box.y1);

    LSceneView tmpView = LSceneView(minimizeStartRect.size() * view->bufferScale(), view->bufferScale());
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

    getView()->enableParentOffset(false);
    tmpView.render();

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

        // Scale and move fullsize view to the dock
        thumbnailFullsizeView->setScalingVector(exp);
        thumbnailFullsizeView->setPos((clickedItem->pos() + clickedItem->size()) * (1.f - exp) +
                 minimizeStartRect.pos() * exp);

        return true;
    },
    [this](LAnimation *)
    {
        setMinimized(false);
    });

    minimizeAnim->start();
}
