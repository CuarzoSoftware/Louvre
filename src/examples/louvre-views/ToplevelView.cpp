#include <LTextureView.h>
#include <LSceneView.h>
#include <LCursor.h>

#include "ToplevelView.h"
#include "Toplevel.h"
#include "Surface.h"
#include "Global.h"

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

    clipBottom = new LLayerView(this);
    surfB = new LSurfaceView(toplevel->surface(), clipBottom);
    surfB->setPrimary(false);
    surfB->enableParentClipping(true);
    surfB->enableCustomPos(true);

    sceneBL = new LSceneView(LSize(22, 22), 2, this);
    sceneBR = new LSceneView(LSize(22, 22), 2, this);

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

    // Masks to make the lower corners of the toplevel round
    maskBL = new LTextureView(G::toplevelTextures().maskBL, sceneBL);
    maskBR = new LTextureView(G::toplevelTextures().maskBR, sceneBR);
    maskBL->setBufferScale(2);
    maskBR->setBufferScale(2);
    maskBL->setPos(0, 1);
    maskBR->setPos(0, 1);

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

void ToplevelView::updateGeometry()
{
    if (toplevel->activated())
    {
        decoTL->setTexture(G::toplevelTextures().activeTL);
        decoT->setTexture(G::toplevelTextures().activeT);
        decoTR->setTexture(G::toplevelTextures().activeTR);
        decoL->setTexture(G::toplevelTextures().activeL);
        decoR->setTexture(G::toplevelTextures().activeR);
        decoBL->setTexture(G::toplevelTextures().activeBL);
        decoB->setTexture(G::toplevelTextures().activeB);
        decoBR->setTexture(G::toplevelTextures().activeBR);
    }
    else
    {
        decoTL->setTexture(G::toplevelTextures().inactiveTL);
        decoT->setTexture(G::toplevelTextures().inactiveT);
        decoTR->setTexture(G::toplevelTextures().inactiveTR);
        decoL->setTexture(G::toplevelTextures().inactiveL);
        decoR->setTexture(G::toplevelTextures().inactiveR);
        decoBL->setTexture(G::toplevelTextures().inactiveBL);
        decoB->setTexture(G::toplevelTextures().inactiveB);
        decoBR->setTexture(G::toplevelTextures().inactiveBR);
    }

    setSize(toplevel->windowGeometry().size());
    clipTop->setSize(size().w(), size().h() - 11);
    class Surface *surf = (class Surface *)toplevel->surface();
    surf->view->setCustomPos(LPoint(-toplevel->windowGeometry().TL().x(), -toplevel->windowGeometry().TL().y()));

    clipBottom->setPos(11, size().h() - 11);
    clipBottom->setSize(size().w() - 2 * 11, 11 - 1);
    surfB->setCustomPos(LPoint(-11 - toplevel->windowGeometry().TL().x(), 11 - size().h() - toplevel->windowGeometry().TL().y()));
    sceneBL->setPos(LPoint(0, size().h() - 11 - 1));
    sceneBR->setPos(LPoint(size().w() - 11, size().h() - 11 - 1));
    surfBL->setCustomPos(LPoint(0, 12 - size().h()));
    surfBR->setCustomPos(LPoint(11 - size().w(), 12 - size().h()));

    decoTL->setPos(-48, -decoTL->size().h());
    decoT->setDstSize(toplevel->windowGeometry().w() - 2 * 11, decoT->texture()->sizeB().h() / 2);
    decoT->setPos(decoTL->nativePos().x() + decoTL->size().w(), -decoT->size().h());
    decoTR->setPos(decoT->nativePos().x() + decoT->size().w(), -decoTR->size().h());
    decoL->setDstSize(decoL->texture()->sizeB().w() / 2, toplevel->windowGeometry().h() - 11 - 1);
    decoL->setPos(-decoL->size().w(), decoTL->nativePos().y() + decoTL->size().h());
    decoR->setDstSize(decoR->texture()->sizeB().w() / 2, toplevel->windowGeometry().h() - 11 - 1);
    decoR->setPos(toplevel->windowGeometry().w(), decoTL->nativePos().y() + decoTL->size().h());
    decoBL->setPos(-48, toplevel->windowGeometry().h() - 11 -1);
    decoBR->setPos(toplevel->windowGeometry().w() - 11, toplevel->windowGeometry().h() - 11 - 1);
    decoB->setDstSize(toplevel->windowGeometry().w() - 2 * 11, decoB->texture()->sizeB().h() / 2);
    decoB->setPos(decoBL->nativePos().x() + decoBL->size().w(), toplevel->windowGeometry().h() - 1);

    // Set topbar opaque/translucent regions
    LRegion transTL;
    transTL.addRect(LRect(0, 0, decoTL->size().w(), decoTL->size().h()));
    transTL.subtractRect(LRect(48, 41, 12, 20));
    decoTL->setTranslucentRegion(&transTL);

    LRegion transT;
    transT.addRect(0, 0, decoT->size().w(), 30);
    decoT->setTranslucentRegion(&transT);

    LRegion transTR;
    transTR.addRect(LRect(0, 0, decoTR->size().w(), decoTR->size().h()));
    transTR.subtractRect(LRect(0, 41, 12, 20));
    decoTR->setTranslucentRegion(&transTR);

    // Set input region
    LRegion inputRegion;
    inputRegion.addRect(LRect(-2, -26, size().w() + 4, 26)); // Topbar
    inputRegion.addRect(LRect(-2, 0, 4, size().h())); // Left
    inputRegion.addRect(LRect(size().w() - 2, 0, 4, size().h())); // Right
    inputRegion.addRect(LRect(-2, size().h() - 2, size().w() + 4, 4)); // Bottom
    setInputRegion(&inputRegion);
    repaint();
}

void ToplevelView::pointerEnterEvent(const LPoint &localPos)
{

}

void ToplevelView::pointerButtonEvent(LPointer::Button button, LPointer::ButtonState state)
{
    if (button != LPointer::Left)
        return;

    seat()->pointer()->setFocus(toplevel->surface());
    seat()->keyboard()->setFocus(toplevel->surface());
    toplevel->configure(toplevel->states() | LToplevelRole::Activated);
    G::compositor()->raiseSurface(toplevel->surface());

    if (state == LPointer::Pressed)
    {
        // Topbar
        if (LRect(3, -26, size().w() - 2, 26).containsPoint(cursor()->pos() - pos()))
            toplevel->startMoveRequest();
        else if (LRect(size().w() - 3, size().h() - 3, 6, 6).containsPoint(cursor()->pos() - pos()))
            toplevel->startResizeRequest(LToplevelRole::ResizeEdge::BottomRight);
    }
}

const LPoint &ToplevelView::nativePos() const
{
    return toplevel->rolePos();
}
