#include <LKeyboardKeyEvent.h>
#include <LTextureView.h>
#include <LSceneView.h>
#include <LAnimation.h>
#include <LCursor.h>
#include <LXCursor.h>
#include <LTime.h>
#include <LLog.h>
#include <LSeat.h>

#include "Compositor.h"
#include "ToplevelButton.h"
#include "ToplevelView.h"
#include "Toplevel.h"
#include "Surface.h"
#include "Global.h"
#include "Pointer.h"
#include "InputRect.h"
#include "Output.h"
#include "TextRenderer.h"
#include "Workspace.h"

static void onPointerLeaveResizeArea(InputRect *rect, void *data);

static void onPointerEnterResizeArea(InputRect *rect, void *data, const LPoint &)
{
    ToplevelView *view = (ToplevelView*)rect->parent();
    Pointer *pointer = (Pointer*)seat()->pointer();
    LXCursor *cursor = (LXCursor*)data;

    if (!seat()->toplevelMoveSessions().empty() || !seat()->toplevelResizeSessions().empty())
        return;

    if (data)
    {
        G::compositor()->cursor()->setTextureB(cursor->texture(), cursor->hotspotB());
        pointer->cursorOwner = view;
    }
    // Topbar input
    else
    {
        G::compositor()->cursor()->useDefault();

        // Hide / show title bar when fullscreen
        if (view->toplevel->fullscreen() && !view->fullscreenTopbarAnim.running() && view->fullscreenTopbarVisibility == 0.f)
        {
            // Stop if the user is interacting with a popup
            if (seat()->keyboard()->grab())
                return;

            view->fullscreenTopbarAnim.setDuration(256);

            view->fullscreenTopbarAnim.setOnUpdateCallback(
            [view](LAnimation *anim)
            {
                view->fullscreenTopbarVisibility = 1.f - powf(1.f - anim->value(), 2.f);
                view->updateGeometry();
                G::compositor()->repaintAllOutputs();
            });

            view->fullscreenTopbarAnim.setOnFinishCallback(
            [view](LAnimation *anim)
            {
                view->fullscreenTopbarVisibility = anim->value();
                view->updateGeometry();

                if (!view->topbarInput.pointerIsOver())
                    onPointerLeaveResizeArea(&view->topbarInput, nullptr);
            });

            view->fullscreenTopbarAnim.start();
        }
    }

    G::compositor()->cursor()->setVisible(true);
}

static void onPointerLeaveResizeArea(InputRect *rect, void *data)
{
    ToplevelView *view = (ToplevelView*)rect->parent();
    Pointer *pointer = (Pointer*)seat()->pointer();

    if (data)
    {
        if (pointer->cursorOwner == view)
        {
            pointer->cursorOwner = nullptr;
            G::compositor()->updatePointerBeforePaint = true;
        }
    }
    else
    {
        if (view->toplevel->fullscreen() && !view->fullscreenTopbarAnim.running() && view->fullscreenTopbarVisibility == 1.f)
        {
            view->fullscreenTopbarAnim.setDuration(256);

            view->fullscreenTopbarAnim.setOnUpdateCallback(
            [view](LAnimation *anim)
            {
                view->fullscreenTopbarVisibility = powf(1.f - anim->value(), 2.f);
                view->updateGeometry();
                if (view->toplevel->fullscreenOutput)
                    view->toplevel->fullscreenOutput->repaint();
            });

            view->fullscreenTopbarAnim.setOnFinishCallback(
            [view](LAnimation *anim)
            {
                view->fullscreenTopbarVisibility = 1.f -anim->value();
                view->updateGeometry();

                if (view->topbarInput.pointerIsOver())
                    onPointerEnterResizeArea(&view->topbarInput, nullptr, LPoint());
            });

            view->fullscreenTopbarAnim.start();
        }
    }
}

static void onPointerButtonResizeArea(InputRect *rect, void *data, UInt32 button, UInt32 state)
{
    ToplevelView *view = (ToplevelView*)rect->parent();
    Pointer *pointer = (Pointer*)seat()->pointer();
    Toplevel *toplevel = view->toplevel;

    if (toplevel->fullscreen())
        return;

    if (button != LPointerButtonEvent::Left)
        return;

    if (state == LPointerButtonEvent::Pressed)
    {
        pointer->setFocus(toplevel->surface());
        seat()->keyboard()->setFocus(toplevel->surface());

        if (!toplevel->activated())
            toplevel->configureState(toplevel->pending().state | LToplevelRole::Activated);
        toplevel->surface()->raise();

        if (data)
            toplevel->startResizeRequest(LPointerButtonEvent(),(LToplevelRole::ResizeEdge)rect->id);
        else
            toplevel->startMoveRequest(LPointerButtonEvent());
    }
    // Maximize / unmaximize on topbar double click
    else if (!data)
    {
        UInt32 now = LTime::ms();

        if (now  - view->lastTopbarClickMs < 300)
        {
            if (view->toplevel->maximized())
                view->toplevel->unsetMaximizedRequest();
            else
                view->toplevel->setMaximizedRequest();
        }

        view->lastTopbarClickMs = now;
    }

    if (pointer->cursorOwner == view)
        pointer->cursorOwner = nullptr;
}

ToplevelView::ToplevelView(Toplevel *toplevel) :
    LLayerView(&G::compositor()->surfacesLayer),
    toplevel(toplevel),
    clipTop(this),
    clipBottom(this),

    // Toplevel decorations (shadows and topbar)
    surfB(toplevel->surface(), &clipBottom),
    sceneBL(LSize(TOPLEVEL_BORDER_RADIUS, TOPLEVEL_BORDER_RADIUS) * 2, 2, this),
    sceneBR(LSize(TOPLEVEL_BORDER_RADIUS, TOPLEVEL_BORDER_RADIUS) * 2, 2, this),
    surfBL(toplevel->surface(), &sceneBL),
    surfBR(toplevel->surface(), &sceneBR),
    decoTL(G::DecorationActiveTL, this),
    decoT(G::DecorationActiveT, this),
    decoTR(G::DecorationActiveTR, this),
    decoL(G::DecorationActiveL, this),
    decoR(G::DecorationActiveR, this),
    decoBL(G::DecorationActiveBL, this),
    decoB(G::DecorationActiveB, this),
    decoBR(G::DecorationActiveBR, this),
    maskBL(G::DecorationMaskBL, &sceneBL),
    maskBR(G::DecorationMaskBR, &sceneBR),
    topbarInput(this, nullptr),
    buttonsContainer(this, this),
    resizeT(this, G::cursors().top_side, LToplevelRole::ResizeEdge::Top),
    resizeB(this, G::cursors().bottom_side, LToplevelRole::ResizeEdge::Bottom),
    resizeL(this, G::cursors().left_side, LToplevelRole::ResizeEdge::Left),
    resizeR(this, G::cursors().right_side, LToplevelRole::ResizeEdge::Right),
    resizeTL(this, G::cursors().top_left_corner, LToplevelRole::ResizeEdge::TopLeft),
    resizeTR(this, G::cursors().top_right_corner, LToplevelRole::ResizeEdge::TopRight),
    resizeBL(this, G::cursors().bottom_left_corner, LToplevelRole::ResizeEdge::BottomLeft),
    resizeBR(this, G::cursors().bottom_right_corner, LToplevelRole::ResizeEdge::BottomRight),
    closeButton(&buttonsContainer, this, ToplevelButton::Close),
    minimizeButton(&buttonsContainer, this, ToplevelButton::Minimize),
    maximizeButton(&buttonsContainer, this, ToplevelButton::Maximize),
    title(nullptr, &topbarInput)
{
    toplevel->decoratedView = this;

    class Surface *surf = toplevel->surf();
    setParent(surf->view.parent());
    surf->view.setPrimary(true);
    surf->view.enableCustomPos(true);
    surf->view.enableParentClipping(true);
    surf->view.setParent(&clipTop);
    surf->view.setCustomPos(LPoint(0, 0));

    surfB.setPrimary(false);
    surfB.enableParentClipping(true);
    surfB.enableCustomPos(true);

    // Make them always translucent
    surfBL.enableCustomTranslucentRegion(true);
    surfBR.enableCustomTranslucentRegion(true);

    // Make them not primary so that the damage of the surface won't be cleared after rendered
    surfBL.setPrimary(false);
    surfBR.setPrimary(false);

    // Ignore the pos given by the surface
    surfBL.enableCustomPos(true);
    surfBR.enableCustomPos(true);

    // Clipped to the bottom border radius rect
    surfBL.enableParentClipping(true);
    surfBR.enableParentClipping(true);

    surfBL.enableParentScaling(false);
    surfBR.enableParentScaling(false);

    // Disable auto blend func
    surfBL.enableAutoBlendFunc(false);
    surfBR.enableAutoBlendFunc(false);

    LBlendFunc blendFunc { GL_ONE, GL_ZERO, GL_ONE, GL_ZERO };
    surfBL.setBlendFunc(blendFunc);
    surfBR.setBlendFunc(blendFunc);

    // Masks to make the lower corners of the toplevel round
    maskBL.setPos(0, 0);
    maskBR.setPos(0, 0);
    maskBL.enableParentScaling(false);
    maskBR.enableParentScaling(false);

    // Disable auto blend func
    maskBL.enableAutoBlendFunc(false);
    maskBR.enableAutoBlendFunc(false);

    // This blending func makes the alpha of the toplevel be replaced by the one of the mask
    blendFunc = { GL_ZERO, GL_SRC_ALPHA, GL_ZERO, GL_SRC_ALPHA };
    maskBL.setBlendFunc(blendFunc);
    maskBR.setBlendFunc(blendFunc);

    resizeTL.onPointerEnter = &onPointerEnterResizeArea;
    resizeTR.onPointerEnter = &onPointerEnterResizeArea;
    resizeBL.onPointerEnter = &onPointerEnterResizeArea;
    resizeBR.onPointerEnter = &onPointerEnterResizeArea;
    resizeT.onPointerEnter = &onPointerEnterResizeArea;
    resizeB.onPointerEnter = &onPointerEnterResizeArea;
    resizeL.onPointerEnter = &onPointerEnterResizeArea;
    resizeR.onPointerEnter = &onPointerEnterResizeArea;
    topbarInput.onPointerEnter = &onPointerEnterResizeArea;

    resizeTL.onPointerLeave = &onPointerLeaveResizeArea;
    resizeTR.onPointerLeave = &onPointerLeaveResizeArea;
    resizeBL.onPointerLeave = &onPointerLeaveResizeArea;
    resizeBR.onPointerLeave = &onPointerLeaveResizeArea;
    resizeT.onPointerLeave = &onPointerLeaveResizeArea;
    resizeB.onPointerLeave = &onPointerLeaveResizeArea;
    resizeL.onPointerLeave = &onPointerLeaveResizeArea;
    resizeR.onPointerLeave = &onPointerLeaveResizeArea;
    topbarInput.onPointerLeave = &onPointerLeaveResizeArea;

    resizeTL.onPointerButton = &onPointerButtonResizeArea;
    resizeTR.onPointerButton = &onPointerButtonResizeArea;
    resizeBL.onPointerButton = &onPointerButtonResizeArea;
    resizeBR.onPointerButton = &onPointerButtonResizeArea;
    resizeT.onPointerButton = &onPointerButtonResizeArea;
    resizeB.onPointerButton = &onPointerButtonResizeArea;
    resizeL.onPointerButton = &onPointerButtonResizeArea;
    resizeR.onPointerButton = &onPointerButtonResizeArea;
    topbarInput.onPointerButton = &onPointerButtonResizeArea;

    // Title label
    title.setBufferScale(2);
    title.setCustomColor({0.1f, 0.1f, 0.1f});
    title.enableCustomColor(true);

    // Buttons
    buttonsContainer.setPos(TOPLEVEL_BUTTON_SPACING, TOPLEVEL_BUTTON_SPACING - TOPLEVEL_TOPBAR_HEIGHT);
    buttonsContainer.setSize(3 * TOPLEVEL_BUTTON_SIZE + 2 * TOPLEVEL_BUTTON_SPACING, TOPLEVEL_BUTTON_SIZE);

    closeButton.enableBlockPointer(false);
    minimizeButton.enableBlockPointer(false);
    maximizeButton.enableBlockPointer(false);

    minimizeButton.setPos(TOPLEVEL_BUTTON_SIZE + TOPLEVEL_BUTTON_SPACING, 0);
    maximizeButton.setPos(2 * (TOPLEVEL_BUTTON_SIZE + TOPLEVEL_BUTTON_SPACING), 0);

    buttonsContainer.onPointerEnter = [](InputRect *, void *data, const LPoint &)
    {
        ToplevelView *view = (ToplevelView*)data;
        Pointer *pointer = (Pointer*)seat()->pointer();

        if (!seat()->toplevelResizeSessions().empty())
            return;

        view->closeButton.update();
        view->minimizeButton.update();
        view->maximizeButton.update();

        if (!pointer->cursorOwner)
            cursor()->useDefault();
    };

    buttonsContainer.onPointerLeave = [](InputRect *, void *data)
    {
        ToplevelView *view = (ToplevelView*)data;

        if (!seat()->toplevelResizeSessions().empty())
            return;

        view->closeButton.update();
        view->minimizeButton.update();
        view->maximizeButton.update();
    };

    // Clip to workspace if parent is fullscreen
    if (parent())
    {
        class Toplevel *tl = G::searchFullscreenParent((class Surface*)toplevel->surface()->parent());

        if (tl)
        {
            setParent(((class Surface*)tl->surface())->getView()->parent());
            enableParentOffset(true);

            for (Workspace *ws : tl->fullscreenOutput->workspaces)
                ws->clipChildren();
        }
    }

    decoL.insertAfter(children().front());
    decoR.insertAfter(children().front());
    decoBL.insertAfter(children().front());
    decoBR.insertAfter(children().front());   
    clipTop.insertAfter(children().front());

    updateGeometry();
}

ToplevelView::~ToplevelView()
{
    fullscreenTopbarAnim.stop();

    Pointer *pointer = (Pointer*)seat()->pointer();

    if (pointer->cursorOwner == this)
    {
        pointer->cursorOwner = nullptr;
        cursor()->useDefault();
    }

    if (title.texture())
        delete title.texture();
}

void ToplevelView::updateTitle()
{
    if (!G::font()->semibold)
        return;

    Int32 maxWidth = (toplevel->windowGeometry().w() - 128) * 2;

    if (title.texture())
    {
        titleWidth = G::font()->semibold->calculateTextureSize(toplevel->title().c_str(), 28).w();

        if (titleWidth != title.texture()->sizeB().w() || titleWidth > maxWidth)
        {
            delete title.texture();
            title.setTexture(G::font()->semibold->renderText(toplevel->title().c_str(), 28, maxWidth));
        }
    }
    else
    {
        title.setTexture(G::font()->semibold->renderText(toplevel->title().c_str(), 28, maxWidth));
    }

    updateGeometry();
}

void ToplevelView::updateGeometry()
{
    class Surface *surf { (class Surface *)toplevel->surface() };

    if (surf->view.parent() != &clipTop)
    {
        surf->view.setParent(&clipTop);
        surf->view.insertAfter(nullptr);
    }

    if (toplevel->windowGeometry().size().area() == 0)
        return;

    int TOPLEVEL_TOP_LEFT_OFFSET_X,
        TOPLEVEL_TOP_LEFT_OFFSET_Y,
        TOPLEVEL_BOTTOM_LEFT_OFFSET_X,
        TOPLEVEL_BOTTOM_LEFT_OFFSET_Y,
        TOPLEVEL_TOP_CLAMP_OFFSET_Y,
        TOPLEVEL_MIN_WIDTH_TOP,
        TOPLEVEL_MIN_WIDTH_BOTTOM,
        TOPLEVEL_MIN_HEIGHT;

    if (toplevel->activated())
    {
        TOPLEVEL_TOP_LEFT_OFFSET_X = TOPLEVEL_ACTIVE_TOP_LEFT_OFFSET_X;
        TOPLEVEL_TOP_LEFT_OFFSET_Y = TOPLEVEL_ACTIVE_TOP_LEFT_OFFSET_Y;
        TOPLEVEL_BOTTOM_LEFT_OFFSET_X = TOPLEVEL_ACTIVE_BOTTOM_LEFT_OFFSET_X;
        TOPLEVEL_BOTTOM_LEFT_OFFSET_Y = TOPLEVEL_ACTIVE_BOTTOM_LEFT_OFFSET_Y;
        TOPLEVEL_TOP_CLAMP_OFFSET_Y = TOPLEVEL_ACTIVE_TOP_CLAMP_OFFSET_Y;
        TOPLEVEL_MIN_WIDTH_TOP = TOPLEVEL_ACTIVE_MIN_WIDTH_TOP;
        TOPLEVEL_MIN_WIDTH_BOTTOM = TOPLEVEL_ACTIVE_MIN_WIDTH_BOTTOM;
        TOPLEVEL_MIN_HEIGHT = TOPLEVEL_ACTIVE_MIN_HEIGHT;

        if (lastActiveState != toplevel->activated())
        {
            title.setCustomColor({0.1f, 0.1f, 0.1f});

            decoTL.setTextureIndex(G::DecorationActiveTL);
            decoT.setTextureIndex(G::DecorationActiveT);
            decoTR.setTextureIndex(G::DecorationActiveTR);
            decoL.setTextureIndex(G::DecorationActiveL);
            decoR.setTextureIndex(G::DecorationActiveR);
            decoBL.setTextureIndex(G::DecorationActiveBL);
            decoB.setTextureIndex(G::DecorationActiveB);
            decoBR.setTextureIndex(G::DecorationActiveBR);

            // Trans region
            decoTL.setTranslucentRegion(&G::toplevelRegions()->activeTransRegionTL);
            decoT.setTranslucentRegion(&G::toplevelRegions()->activeTransRegionTR);
        }
    }
    else
    {
        TOPLEVEL_TOP_LEFT_OFFSET_X = TOPLEVEL_INACTIVE_TOP_LEFT_OFFSET_X;
        TOPLEVEL_TOP_LEFT_OFFSET_Y = TOPLEVEL_INACTIVE_TOP_LEFT_OFFSET_Y;
        TOPLEVEL_BOTTOM_LEFT_OFFSET_X = TOPLEVEL_INACTIVE_BOTTOM_LEFT_OFFSET_X;
        TOPLEVEL_BOTTOM_LEFT_OFFSET_Y = TOPLEVEL_INACTIVE_BOTTOM_LEFT_OFFSET_Y;
        TOPLEVEL_TOP_CLAMP_OFFSET_Y = TOPLEVEL_INACTIVE_TOP_CLAMP_OFFSET_Y;
        TOPLEVEL_MIN_WIDTH_TOP = TOPLEVEL_INACTIVE_MIN_WIDTH_TOP;
        TOPLEVEL_MIN_WIDTH_BOTTOM = TOPLEVEL_INACTIVE_MIN_WIDTH_BOTTOM;
        TOPLEVEL_MIN_HEIGHT = TOPLEVEL_INACTIVE_MIN_HEIGHT;

        if (lastActiveState != toplevel->activated())
        {
            title.setCustomColor({0.7f, 0.7f, 0.7f});

            decoTL.setTextureIndex(G::DecorationInactiveTL);
            decoT.setTextureIndex(G::DecorationInactiveT);
            decoTR.setTextureIndex(G::DecorationInactiveTR);
            decoL.setTextureIndex(G::DecorationInactiveL);
            decoR.setTextureIndex(G::DecorationInactiveR);
            decoBL.setTextureIndex(G::DecorationInactiveBL);
            decoB.setTextureIndex(G::DecorationInactiveB);
            decoBR.setTextureIndex(G::DecorationInactiveBR);

            // Trans region
            decoTL.setTranslucentRegion(&G::toplevelRegions()->inactiveTransRegionTL);
            decoT.setTranslucentRegion(&G::toplevelRegions()->inactiveTransRegionTR);
        }
    }

    // Update titlebar button icons
    closeButton.update();
    minimizeButton.update();
    maximizeButton.update();
    lastActiveState = toplevel->activated();

    if (toplevel->fullscreen())
    {
        if (!toplevel->fullscreenOutput)
            return;

        if (!lastFullscreenState)
        {
            clipBottom.setVisible(false);
            sceneBL.setVisible(false);
            sceneBR.setVisible(false);
            maskBL.setVisible(false);
            maskBR.setVisible(false);
            decoTL.setVisible(false);
            decoTR.setVisible(false);
            decoL.setVisible(false);
            decoR.setVisible(false);
            decoBL.setVisible(false);
            decoBR.setVisible(false);
            decoB.setVisible(false);
            resizeT.setVisible(false);
            resizeB.setVisible(false);
            resizeL.setVisible(false);
            resizeR.setVisible(false);
            resizeTL.setVisible(false);
            resizeTR.setVisible(false);
            resizeBL.setVisible(false);
            resizeBR.setVisible(false);
            buttonsContainer.enableBlockPointer(false);
        }

        setSize(toplevel->fullscreenOutput->size());

        surf->view.setCustomPos((toplevel->fullscreenOutput->size() - toplevel->windowGeometry().size()) / 2);

        const LSize size { nativeSize() };

        clipTop.setSize(size);
        decoT.setDstSize(size.w(), decoT.nativeSize().h());
        decoT.setPos(0, -(decoT.nativeSize().h() + 1) + (TOPLEVEL_TOPBAR_HEIGHT + TOPLEVEL_TOP_CLAMP_OFFSET_Y) * fullscreenTopbarVisibility);
        buttonsContainer.setPos(TOPLEVEL_BUTTON_SPACING, TOPLEVEL_BUTTON_SPACING - TOPLEVEL_TOPBAR_HEIGHT * (1.f - fullscreenTopbarVisibility));

        // Set topbar center translucent regions
        LRegion transT;
        transT.addRect(
            0,
            0,
            decoT.size().w(),
            decoT.size().h() - TOPLEVEL_TOPBAR_HEIGHT - TOPLEVEL_TOP_CLAMP_OFFSET_Y);
        transT.addRect(
            0,
            decoT.size().h() - TOPLEVEL_TOP_CLAMP_OFFSET_Y,
            decoT.size().w(),
            TOPLEVEL_TOP_CLAMP_OFFSET_Y);

        decoT.setTranslucentRegion(&transT);

        topbarInput.setPos(0, - (TOPLEVEL_TOPBAR_HEIGHT) * (1.f - fullscreenTopbarVisibility));
        topbarInput.setSize(size.w(), TOPLEVEL_TOPBAR_HEIGHT + 1);
    }
    else
    {
        if (lastFullscreenState)
        {
            buttonsContainer.setPos(TOPLEVEL_BUTTON_SPACING, TOPLEVEL_BUTTON_SPACING - TOPLEVEL_TOPBAR_HEIGHT);
            surf->view.setCustomPos(0, 0);
            clipBottom.setVisible(true);
            sceneBL.setVisible(true);
            sceneBR.setVisible(true);
            maskBL.setVisible(true);
            maskBR.setVisible(true);
            decoTL.setVisible(true);
            decoT.setVisible(true);
            decoTR.setVisible(true);
            decoL.setVisible(true);
            decoR.setVisible(true);
            decoBL.setVisible(true);
            decoBR.setVisible(true);
            decoB.setVisible(true);
            resizeT.setVisible(true);
            resizeB.setVisible(true);
            resizeL.setVisible(true);
            resizeR.setVisible(true);
            resizeTL.setVisible(true);
            resizeTR.setVisible(true);
            resizeBL.setVisible(true);
            resizeBR.setVisible(true);
            buttonsContainer.enableBlockPointer(true);
        }

        const Int32 clip { 1 };
        const LSize tlSize { toplevel->windowGeometry().size().w() - 2 * clip, toplevel->windowGeometry().size().h() - 2 * clip };

        setSize(
            tlSize.w() < 150 ? 149 : tlSize.w(),
            tlSize.h() < 150 ? 149 : tlSize.h());

        const LSize size { nativeSize() };

        // Upper surface view
        clipTop.setSize(
            size.w(),
            size.h() - TOPLEVEL_BORDER_RADIUS);
        surf->view.setCustomPos(- clip, - clip);

        // Lower surface view (without border radius rects)
        clipBottom.setPos(
            TOPLEVEL_BORDER_RADIUS,
            size.h() - TOPLEVEL_BORDER_RADIUS);
        clipBottom.setSize(
            size.w() - 2 * TOPLEVEL_BORDER_RADIUS,
            TOPLEVEL_BORDER_RADIUS);
        surfB.setCustomPos(
            - TOPLEVEL_BORDER_RADIUS - clip,
            TOPLEVEL_BORDER_RADIUS - size.h() - clip);

        // Bottom left / right surfaces views
        sceneBL.setPos(
            0,
            size.h() - TOPLEVEL_BORDER_RADIUS);
        sceneBR.setPos(
            size.w() - TOPLEVEL_BORDER_RADIUS,
            size.h() - TOPLEVEL_BORDER_RADIUS);
        surfBL.setCustomPos(
            - clip,
            TOPLEVEL_BORDER_RADIUS - size.h() - clip);
        surfBR.setCustomPos(
            TOPLEVEL_BORDER_RADIUS - size.w() - clip,
            TOPLEVEL_BORDER_RADIUS - size.h() - clip);

        // Decorations
        decoTL.setPos(
            TOPLEVEL_TOP_LEFT_OFFSET_X,
            TOPLEVEL_TOP_LEFT_OFFSET_Y);
        decoT.setDstSize(
            size.w() - TOPLEVEL_MIN_WIDTH_TOP,
            decoT.nativeSize().h());
        decoT.setPos(
            decoTL.nativePos().x() + decoTL.nativeSize().w(),
            -decoT.nativeSize().h() + TOPLEVEL_TOP_CLAMP_OFFSET_Y);
        decoTR.setPos(
            size.w() - decoTR.nativeSize().w() - TOPLEVEL_TOP_LEFT_OFFSET_X,
            TOPLEVEL_TOP_LEFT_OFFSET_Y);
        decoL.setDstSize(
            decoL.nativeSize().w(),
            size.h() - TOPLEVEL_MIN_HEIGHT);
        decoL.setPos(
            -decoL.nativeSize().w(),
            decoTL.nativePos().y() + decoTL.nativeSize().h());
        decoR.setDstSize(
            decoR.nativeSize().w(),
            size.h() - TOPLEVEL_MIN_HEIGHT);
        decoR.setPos(
            size.w(),
            decoTL.nativePos().y() + decoTL.nativeSize().h());
        decoBL.setPos(
            TOPLEVEL_BOTTOM_LEFT_OFFSET_X,
            size.h() + TOPLEVEL_BOTTOM_LEFT_OFFSET_Y);
        decoBR.setPos(
            size.w() - decoBR.nativeSize().w() - TOPLEVEL_BOTTOM_LEFT_OFFSET_X,
            size.h() + TOPLEVEL_BOTTOM_LEFT_OFFSET_Y);
        decoB.setDstSize(
            size.w() - TOPLEVEL_MIN_WIDTH_BOTTOM,
            decoB.nativeSize().h());
        decoB.setPos(
            decoBL.nativePos().x() + decoBL.nativeSize().w(),
            size.h());

        // Set topbar center translucent regions
        LRegion transT;
        transT.addRect(
            0,
            0,
            decoT.size().w(),
            decoT.size().h() - TOPLEVEL_TOPBAR_HEIGHT - TOPLEVEL_TOP_CLAMP_OFFSET_Y + 1);
        transT.addRect( // Bottom line
            0,
            decoT.size().h() - TOPLEVEL_TOP_CLAMP_OFFSET_Y,
            decoT.size().w(),
            TOPLEVEL_TOP_CLAMP_OFFSET_Y);

        decoT.setTranslucentRegion(&transT);

        // Update input rects
        resizeT.setPos(0, - TOPLEVEL_TOPBAR_HEIGHT - TOPLEVEL_RESIZE_INPUT_MARGIN);
        resizeT.setSize(size.w(), TOPLEVEL_RESIZE_INPUT_MARGIN * 2);

        resizeB.setPos(0, size.h() - TOPLEVEL_RESIZE_INPUT_MARGIN);
        resizeB.setSize(size.w(), TOPLEVEL_RESIZE_INPUT_MARGIN * 2);

        resizeL.setPos(-TOPLEVEL_RESIZE_INPUT_MARGIN, - TOPLEVEL_TOPBAR_HEIGHT);
        resizeL.setSize(TOPLEVEL_RESIZE_INPUT_MARGIN * 2, size.h() + TOPLEVEL_TOPBAR_HEIGHT);

        resizeR.setPos(size.w() - TOPLEVEL_RESIZE_INPUT_MARGIN, - TOPLEVEL_TOPBAR_HEIGHT);
        resizeR.setSize(TOPLEVEL_RESIZE_INPUT_MARGIN * 2, size.h() + TOPLEVEL_TOPBAR_HEIGHT);

        resizeTL.setPos(-TOPLEVEL_RESIZE_INPUT_MARGIN, - TOPLEVEL_TOPBAR_HEIGHT - TOPLEVEL_RESIZE_INPUT_MARGIN);
        resizeTL.setSize(TOPLEVEL_RESIZE_INPUT_MARGIN + TOPLEVEL_BORDER_RADIUS, TOPLEVEL_RESIZE_INPUT_MARGIN + TOPLEVEL_BORDER_RADIUS);

        resizeTR.setPos(size.w() - TOPLEVEL_BORDER_RADIUS, - TOPLEVEL_TOPBAR_HEIGHT - TOPLEVEL_RESIZE_INPUT_MARGIN);
        resizeTR.setSize(TOPLEVEL_RESIZE_INPUT_MARGIN + TOPLEVEL_BORDER_RADIUS, TOPLEVEL_RESIZE_INPUT_MARGIN + TOPLEVEL_BORDER_RADIUS);

        resizeBL.setPos(-TOPLEVEL_RESIZE_INPUT_MARGIN, size.h() - TOPLEVEL_BORDER_RADIUS);
        resizeBL.setSize(TOPLEVEL_RESIZE_INPUT_MARGIN + TOPLEVEL_BORDER_RADIUS, TOPLEVEL_RESIZE_INPUT_MARGIN + TOPLEVEL_BORDER_RADIUS);

        resizeBR.setPos(size.w() - TOPLEVEL_BORDER_RADIUS, size.h() - TOPLEVEL_BORDER_RADIUS);
        resizeBR.setSize(TOPLEVEL_RESIZE_INPUT_MARGIN + TOPLEVEL_BORDER_RADIUS, TOPLEVEL_RESIZE_INPUT_MARGIN + TOPLEVEL_BORDER_RADIUS);

        topbarInput.setPos(0, - TOPLEVEL_TOPBAR_HEIGHT);
        topbarInput.setSize(size.w(), TOPLEVEL_TOPBAR_HEIGHT - 1);
    }

    // Update title pos
    if (title.texture())
    {
        Int32 px { (topbarInput.size().w() - title.size().w()) / 2 };

        if (titleWidth > (topbarInput.size().w() - 128) * 2)
            px = 64;

        Int32 h { topbarInput.size().h() - (TOPLEVEL_TOPBAR_HEIGHT + title.size().h()) / 2 };

        if (toplevel->fullscreen())
            h -= 2;

        title.setPos(px, h);
    }

    lastFullscreenState = toplevel->fullscreen();

}

bool ToplevelView::nativeMapped() const noexcept
{
    return toplevel->surface()->mapped();
}

const LPoint &ToplevelView::nativePos() const noexcept
{
    return toplevel->rolePos();
}

void ToplevelView::keyEvent(const LKeyboardKeyEvent &event)
{
    if (event.keyCode() == KEY_LEFTALT)
        maximizeButton.update();
}
