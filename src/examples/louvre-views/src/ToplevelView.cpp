#include <LTextureView.h>
#include <LSceneView.h>
#include <LCursor.h>
#include <LXCursor.h>

#include "ToplevelView.h"
#include "Toplevel.h"
#include "Surface.h"
#include "Global.h"
#include "Pointer.h"

ToplevelView::ToplevelView(Toplevel *toplevel) : LLayerView(G::compositor()->surfacesLayer)
{
    this->toplevel = toplevel;
    toplevel->decoratedView = this;
    enableInput(true);

    clipTop = new LLayerView(this);
    clipTop->setPos(0, 0);
    class Surface *surf = (class Surface*)toplevel->surface();
    surf->view->setPrimary(true);
    surf->view->enableCustomPos(true);
    surf->view->enableParentClipping(true);
    surf->view->setParent(clipTop);
    surf->view->setCustomPos(LPoint(0, 0));
    surf->view->enableBlockPointer(false);

    clipBottom = new LLayerView(this);
    surfB = new LSurfaceView(toplevel->surface(), clipBottom);
    surfB->setPrimary(false);
    surfB->enableParentClipping(true);
    surfB->enableCustomPos(true);
    surfB->enableBlockPointer(false);

    sceneBL = new LSceneView(LSize(TOPLEVEL_BORDER_RADIUS, TOPLEVEL_BORDER_RADIUS) * 2, 2, this);
    sceneBR = new LSceneView(LSize(TOPLEVEL_BORDER_RADIUS, TOPLEVEL_BORDER_RADIUS) * 2, 2, this);

    surfBL = new LSurfaceView(toplevel->surface(), sceneBL);
    surfBR = new LSurfaceView(toplevel->surface(), sceneBR);

    // Make them always translucent
    surfBL->enableCustomTranslucentRegion(true);
    surfBR->enableCustomTranslucentRegion(true);

    // Make them not primary so that the damage of the surface won't be cleared after rendered
    surfBL->setPrimary(false);
    surfBR->setPrimary(false);

    // Ignore the pos given by the surface
    surfBL->enableCustomPos(true);
    surfBR->enableCustomPos(true);

    // Clipped to the bottom border radius rect
    surfBL->enableParentClipping(true);
    surfBR->enableParentClipping(true);

    // Let input events get to parent view
    surfBL->enableBlockPointer(false);
    surfBR->enableBlockPointer(false);

    // Masks to make the lower corners of the toplevel round
    maskBL = new LTextureView(G::toplevelTextures().maskBL, sceneBL);
    maskBR = new LTextureView(G::toplevelTextures().maskBR, sceneBR);
    maskBL->setBufferScale(2);
    maskBR->setBufferScale(2);
    maskBL->setPos(0, 0);
    maskBR->setPos(0, 0);

    // This blending func makes the alpha of the toplevel be replaced by the one of the mask
    maskBL->setBlendFunc(GL_ZERO, GL_SRC_ALPHA);
    maskBR->setBlendFunc(GL_ZERO, GL_SRC_ALPHA);

    // Toplevel decorations (shadows and topbar)
    decoTL = new LTextureView(G::toplevelTextures().activeTL, this);
    decoT = new LTextureView(G::toplevelTextures().activeT, this);
    decoTR = new LTextureView(G::toplevelTextures().activeTR, this);
    decoL = new LTextureView(G::toplevelTextures().activeL, this);
    decoR = new LTextureView(G::toplevelTextures().activeR, this);
    decoBL = new LTextureView(G::toplevelTextures().activeBL, this);
    decoB = new LTextureView(G::toplevelTextures().activeB, this);
    decoBR = new LTextureView(G::toplevelTextures().activeBR, this);

    decoTL->setBufferScale(2);
    decoT->setBufferScale(2);
    decoTR->setBufferScale(2);
    decoL->setBufferScale(2);
    decoR->setBufferScale(2);
    decoBL->setBufferScale(2);
    decoB->setBufferScale(2);
    decoBR->setBufferScale(2);

    decoT->enableDstSize(true);
    decoL->enableDstSize(true);
    decoR->enableDstSize(true);
    decoB->enableDstSize(true);

    updateGeometry();
}

ToplevelView::~ToplevelView()
{
    delete clipTop;
    delete clipBottom;
    delete surfB;
    delete decoTL;
    delete decoT;
    delete decoTR;
    delete decoL;
    delete decoR;
    delete decoBL;
    delete decoB;
    delete decoBR;
    delete maskBL;
    delete maskBR;
    delete sceneBL;
    delete sceneBR;
    delete surfBL;
    delete surfBR;
}

void ToplevelView::updateGeometry()
{
    class Surface *surf = (class Surface *)toplevel->surface();

    if (surf->view->parent() != clipTop)
    {
        surf->view->setParent(clipTop);
        surf->view->insertAfter(nullptr);
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
            decoTL->setTexture(G::toplevelTextures().activeTL);
            decoT->setTexture(G::toplevelTextures().activeT);
            decoTR->setTexture(G::toplevelTextures().activeTR);
            decoL->setTexture(G::toplevelTextures().activeL);
            decoR->setTexture(G::toplevelTextures().activeR);
            decoBL->setTexture(G::toplevelTextures().activeBL);
            decoB->setTexture(G::toplevelTextures().activeB);
            decoBR->setTexture(G::toplevelTextures().activeBR);

            // Trans region
            decoTL->setTranslucentRegion(&G::toplevelTextures().activeTransRegionTL);
            decoTR->setTranslucentRegion(&G::toplevelTextures().activeTransRegionTR);
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
            decoTL->setTexture(G::toplevelTextures().inactiveTL);
            decoT->setTexture(G::toplevelTextures().inactiveT);
            decoTR->setTexture(G::toplevelTextures().inactiveTR);
            decoL->setTexture(G::toplevelTextures().inactiveL);
            decoR->setTexture(G::toplevelTextures().inactiveR);
            decoBL->setTexture(G::toplevelTextures().inactiveBL);
            decoB->setTexture(G::toplevelTextures().inactiveB);
            decoBR->setTexture(G::toplevelTextures().inactiveBR);

            // Trans region
            decoTL->setTranslucentRegion(&G::toplevelTextures().inactiveTransRegionTL);
            decoTR->setTranslucentRegion(&G::toplevelTextures().inactiveTransRegionTR);
        }
    }

    lastActiveState = toplevel->activated();

    Int32 clip = 1;

    setSize(
        toplevel->windowGeometry().size().w() - 2 * clip,
        toplevel->windowGeometry().size().h() - 2 * clip);

    // Upper surface view
    clipTop->setSize(
        size().w(),
        size().h() - TOPLEVEL_BORDER_RADIUS);
    surf->view->setCustomPos(- clip, - clip);

    // Lower surface view (without border radius rects)
    clipBottom->setPos(
        TOPLEVEL_BORDER_RADIUS,
        size().h() - TOPLEVEL_BORDER_RADIUS);
    clipBottom->setSize(
        size().w() - 2 * TOPLEVEL_BORDER_RADIUS,
        TOPLEVEL_BORDER_RADIUS);
    surfB->setCustomPos(
        - TOPLEVEL_BORDER_RADIUS - clip,
        TOPLEVEL_BORDER_RADIUS - size().h() - clip);

    // Bottom left / right surfaces views
    sceneBL->setPos(
        0,
        size().h() - TOPLEVEL_BORDER_RADIUS);
    sceneBR->setPos(
        size().w() - TOPLEVEL_BORDER_RADIUS,
        size().h() - TOPLEVEL_BORDER_RADIUS);
    surfBL->setCustomPos(
        - clip,
        TOPLEVEL_BORDER_RADIUS - size().h() - clip);
    surfBR->setCustomPos(
        TOPLEVEL_BORDER_RADIUS - size().w() - clip,
        TOPLEVEL_BORDER_RADIUS - size().h() - clip);

    // Decorations
    decoTL->setPos(
        TOPLEVEL_TOP_LEFT_OFFSET_X,
        TOPLEVEL_TOP_LEFT_OFFSET_Y);
    decoT->setDstSize(
        size().w() - TOPLEVEL_MIN_WIDTH_TOP,
        decoT->texture()->sizeB().h() / 2);
    decoT->setPos(
        decoTL->nativePos().x() + decoTL->size().w(),
        -decoT->size().h() + TOPLEVEL_TOP_CLAMP_OFFSET_Y);
    decoTR->setPos(
        size().w() - decoTR->size().w() - TOPLEVEL_TOP_LEFT_OFFSET_X,
        TOPLEVEL_TOP_LEFT_OFFSET_Y);
    decoL->setDstSize(
        decoL->texture()->sizeB().w() / 2,
        size().h() - TOPLEVEL_MIN_HEIGHT);
    decoL->setPos(
        -decoL->size().w(),
        decoTL->nativePos().y() + decoTL->size().h());
    decoR->setDstSize(
        decoR->texture()->sizeB().w() / 2,
        size().h() - TOPLEVEL_MIN_HEIGHT);
    decoR->setPos(
        size().w(),
        decoTL->nativePos().y() + decoTL->size().h());
    decoBL->setPos(
        TOPLEVEL_BOTTOM_LEFT_OFFSET_X,
        size().h() + TOPLEVEL_BOTTOM_LEFT_OFFSET_Y);
    decoBR->setPos(
        size().w() - decoBR->size().w() - TOPLEVEL_BOTTOM_LEFT_OFFSET_X,
        size().h() + TOPLEVEL_BOTTOM_LEFT_OFFSET_Y);
    decoB->setDstSize(
        size().w() - TOPLEVEL_MIN_WIDTH_BOTTOM,
        decoB->texture()->sizeB().h() / 2);
    decoB->setPos(
        decoBL->nativePos().x() + decoBL->size().w(),
        size().h());

    // Set topbar center translucent regions
    LRegion transT;
    transT.addRect(
        0,
        0,
        decoT->size().w(),
        decoT->size().h() - TOPLEVEL_TOPBAR_HEIGHT - TOPLEVEL_TOP_CLAMP_OFFSET_Y);
    transT.addRect(
        0,
        decoT->size().h() - TOPLEVEL_TOP_CLAMP_OFFSET_Y,
        decoT->size().w(),
        TOPLEVEL_TOP_CLAMP_OFFSET_Y);
    decoT->setTranslucentRegion(&transT);


    // Set input region
    LRegion inputRegion;

    // Top input rect
    inputRegion.addRect(-TOPLEVEL_RESIZE_INPUT_MARGIN,
                        -TOPLEVEL_TOPBAR_HEIGHT - TOPLEVEL_RESIZE_INPUT_MARGIN,
                        size().w() + 2 * TOPLEVEL_RESIZE_INPUT_MARGIN,
                        size().h() + TOPLEVEL_TOPBAR_HEIGHT + 2 * TOPLEVEL_RESIZE_INPUT_MARGIN);

    setInputRegion(&inputRegion);

    repaint();
}

void ToplevelView::pointerEnterEvent(const LPoint &localPos)
{
    L_UNUSED(localPos);

    Pointer *pointer = (Pointer*)seat()->pointer();
    pointer->restoreCursor = false;
}

void ToplevelView::pointerLeaveEvent()
{
    Pointer *pointer = (Pointer*)seat()->pointer();

    if (pointer->resizingToplevel())
        return;

    pointer->restoreCursor = true;
}

void ToplevelView::pointerMoveEvent(const LPoint &localPos)
{
    Pointer *pointer = (Pointer*)seat()->pointer();
    pointer->restoreCursor = false;

    if (pointer->resizingToplevel())
        return;

    if (localPos.x() < TOPLEVEL_RESIZE_INPUT_MARGIN)
    {
        if (G::cursors().top_left_corner && localPos.y() < - TOPLEVEL_TOPBAR_HEIGHT + TOPLEVEL_RESIZE_INPUT_MARGIN)
            cursor()->setTextureB(G::cursors().top_left_corner->texture(), G::cursors().top_left_corner->hotspotB());
        else if (G::cursors().bottom_left_corner && localPos.y() > size().h() - TOPLEVEL_RESIZE_INPUT_MARGIN)
            cursor()->setTextureB(G::cursors().bottom_left_corner->texture(), G::cursors().bottom_left_corner->hotspotB());
        else if (G::cursors().left_side)
            cursor()->setTextureB(G::cursors().left_side->texture(), G::cursors().left_side->hotspotB());
    }
    else if (localPos.x() > size().w() - TOPLEVEL_RESIZE_INPUT_MARGIN)
    {
        if (G::cursors().top_right_corner && localPos.y() < - TOPLEVEL_TOPBAR_HEIGHT + TOPLEVEL_RESIZE_INPUT_MARGIN)
            cursor()->setTextureB(G::cursors().top_right_corner->texture(), G::cursors().top_right_corner->hotspotB());
        else if (G::cursors().bottom_right_corner && localPos.y() > size().h() - TOPLEVEL_RESIZE_INPUT_MARGIN)
            cursor()->setTextureB(G::cursors().bottom_right_corner->texture(), G::cursors().bottom_right_corner->hotspotB());
        else if (G::cursors().right_side)
            cursor()->setTextureB(G::cursors().right_side->texture(), G::cursors().right_side->hotspotB());
    }
    else
    {
        if (G::cursors().top_side && localPos.y() < - TOPLEVEL_TOPBAR_HEIGHT + TOPLEVEL_RESIZE_INPUT_MARGIN)
            cursor()->setTextureB(G::cursors().top_side->texture(), G::cursors().top_side->hotspotB());
        else if (G::cursors().bottom_side && localPos.y() > size().h() - TOPLEVEL_RESIZE_INPUT_MARGIN)
            cursor()->setTextureB(G::cursors().bottom_side->texture(), G::cursors().bottom_side->hotspotB());
        else
            cursor()->useDefault();
    }

    cursor()->setVisible(true);
}

void ToplevelView::pointerButtonEvent(LPointer::Button button, LPointer::ButtonState state)
{
    if (button != LPointer::Left)
        return;

    if (state == LPointer::Pressed)
    {
        seat()->pointer()->setFocus(toplevel->surface());
        seat()->keyboard()->setFocus(toplevel->surface());
        toplevel->configure(toplevel->states() | LToplevelRole::Activated);
        G::compositor()->raiseSurface(toplevel->surface());

        LPoint localPos = cursor()->pos() - pos();

        if (localPos.x() < TOPLEVEL_RESIZE_INPUT_MARGIN)
        {
            if (localPos.y() < - 30 + TOPLEVEL_RESIZE_INPUT_MARGIN)
                toplevel->startResizeRequest(LToplevelRole::ResizeEdge::TopLeft);
            else if (localPos.y() > size().h() - TOPLEVEL_RESIZE_INPUT_MARGIN)
                toplevel->startResizeRequest(LToplevelRole::ResizeEdge::BottomLeft);
            else
                toplevel->startResizeRequest(LToplevelRole::ResizeEdge::Left);
        }
        else if (localPos.x() > size().w() - TOPLEVEL_RESIZE_INPUT_MARGIN)
        {
            if (localPos.y() < - 30 + TOPLEVEL_RESIZE_INPUT_MARGIN)
                toplevel->startResizeRequest(LToplevelRole::ResizeEdge::TopRight);
            else if (localPos.y() > size().h() - TOPLEVEL_RESIZE_INPUT_MARGIN)
                toplevel->startResizeRequest(LToplevelRole::ResizeEdge::BottomRight);
            else
                toplevel->startResizeRequest(LToplevelRole::ResizeEdge::Right);
        }
        else
        {
            if (localPos.y() < - 30 + TOPLEVEL_RESIZE_INPUT_MARGIN)
                toplevel->startResizeRequest(LToplevelRole::ResizeEdge::Top);
            else if (localPos.y() > size().h() - TOPLEVEL_RESIZE_INPUT_MARGIN)
                toplevel->startResizeRequest(LToplevelRole::ResizeEdge::Bottom);
            else
                toplevel->startMoveRequest();
        }
    }
}

bool ToplevelView::nativeMapped() const
{
    return toplevel->surface()->mapped();
}

const LPoint &ToplevelView::nativePos() const
{
    return toplevel->rolePos();
}
