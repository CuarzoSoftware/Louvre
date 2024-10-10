#include <LLayerRole.h>
#include <LCursor.h>
#include <LSurface.h>
#include <LCompositor.h>
#include <LSeat.h>
#include <LKeyboard.h>

using namespace Louvre;

//! [rolePos]
const LPoint &LLayerRole::rolePos() const
{
    // If no output has been assigned, use the user-defined position.
    if (!exclusiveOutput())
        return surface()->pos();

    if (anchor() == (LEdgeTop | LEdgeLeft))
    {
        m_rolePos = exclusiveOutput()->pos() + exclusiveZone().rect().pos();
    }
    else if (anchor() == (LEdgeTop | LEdgeRight))
    {
        m_rolePos.setX(exclusiveOutput()->pos().x() + exclusiveZone().rect().x()
                       + exclusiveZone().rect().w() - surface()->size().w());
        m_rolePos.setY(exclusiveOutput()->pos().y() + exclusiveZone().rect().y());
    }
    else if (anchor() == (LEdgeBottom | LEdgeRight))
    {
        m_rolePos.setX(exclusiveOutput()->pos().x() + exclusiveZone().rect().x()
                       + exclusiveZone().rect().w() - surface()->size().w());
        m_rolePos.setY(exclusiveOutput()->pos().y() + exclusiveZone().rect().y()
                       + exclusiveZone().rect().h() - surface()->size().h());
    }
    else if (anchor() == (LEdgeBottom | LEdgeLeft))
    {
        m_rolePos.setX(exclusiveOutput()->pos().x() + exclusiveZone().rect().x());
        m_rolePos.setY(exclusiveOutput()->pos().y() + exclusiveZone().rect().y()
                       + exclusiveZone().rect().h() - surface()->size().h());
    }
    else if (anchor() == LEdgeTop || anchor() == (LEdgeLeft | LEdgeTop | LEdgeRight))
    {
        m_rolePos.setX(exclusiveOutput()->pos().x() + exclusiveZone().rect().x()
                       + (exclusiveZone().rect().w() - surface()->size().w()) / 2 );
        m_rolePos.setY(exclusiveOutput()->pos().y() + exclusiveZone().rect().y());
    }
    else if (anchor() == LEdgeBottom || anchor() == (LEdgeLeft | LEdgeBottom | LEdgeRight))
    {
        m_rolePos.setX(exclusiveOutput()->pos().x() + exclusiveZone().rect().x()
                       + (exclusiveZone().rect().w() - surface()->size().w()) / 2 );
        m_rolePos.setY(exclusiveOutput()->pos().y() + exclusiveZone().rect().y()
                       + exclusiveZone().rect().h() - surface()->size().h());
    }
    else if (anchor() == LEdgeLeft || anchor() == (LEdgeBottom | LEdgeLeft | LEdgeTop))
    {
        m_rolePos.setX(exclusiveOutput()->pos().x() + exclusiveZone().rect().x());
        m_rolePos.setY(exclusiveOutput()->pos().y() + exclusiveZone().rect().y()
                       + (exclusiveZone().rect().h() - surface()->size().h()) / 2);
    }
    else if (anchor() == LEdgeRight || anchor() == (LEdgeBottom | LEdgeRight | LEdgeTop))
    {
        m_rolePos.setX(exclusiveOutput()->pos().x() + exclusiveZone().rect().x()
                       + exclusiveZone().rect().w() - surface()->size().w());
        m_rolePos.setY(exclusiveOutput()->pos().y() + exclusiveZone().rect().y()
                       + (exclusiveZone().rect().h() - surface()->size().h()) / 2);
    }
    else
        m_rolePos = exclusiveOutput()->pos() + exclusiveZone().rect().pos() + (exclusiveZone().rect().size() - surface()->size()) / 2;

    // Add extra margins.
    if (anchor().check(LEdgeTop))
        m_rolePos.setY(m_rolePos.y() + margins().top);
    if (anchor().check(LEdgeBottom))
        m_rolePos.setY(m_rolePos.y() - margins().bottom);
    if (anchor().check(LEdgeLeft))
        m_rolePos.setX(m_rolePos.x() + margins().left);
    if (anchor().check(LEdgeRight))
        m_rolePos.setX(m_rolePos.x() - margins().right);

    return m_rolePos;
}
//! [rolePos]

//! [atomsChanged]
void LLayerRole::atomsChanged(LBitset<AtomChanges> changes, const Atoms &prevAtoms)
{
    L_UNUSED(prevAtoms);

    if (changes.check(KeyboardInteractivityChanged))
    {
        switch (keyboardInteractivity())
        {
        case KeyboardInteractivity::NoInteractivity:
            if (surface()->hasKeyboardFocus())
                seat()->keyboard()->setFocus(nullptr);
            if (surface()->hasKeyboardGrab())
                seat()->keyboard()->setGrab(nullptr);
            break;
        case KeyboardInteractivity::OnDemand:
            if (surface()->hasKeyboardGrab())
                seat()->keyboard()->setGrab(nullptr);
            break;
        case KeyboardInteractivity::Exclusive:
            seat()->keyboard()->setGrab(surface());
            break;
        }
    }

    if (changes.check(MarginsChanged))
        surface()->requestNextFrame(false);

    if (exclusiveOutput() && changes.check(
            ExclusiveEdgeChanged | ExclusiveZoneSizeChanged |
            SizeChanged | AnchorChanged | LayerChanged | MarginsChanged))
        exclusiveOutput()->repaint();
}
//! [atomsChanged]

//! [configureRequest]
void LLayerRole::configureRequest()
{
    // If exclusive output is not set, assign the cursor's current output.
    if (!exclusiveOutput())
        setExclusiveOutput(cursor()->output());

    surface()->sendOutputEnterEvent(exclusiveOutput());

    // The client-suggested size.
    LSize newSize { size() };

    /*
     * LExclusiveZone calculates the single edge the surface is anchored to.
     * It can also return 0 if the anchor() flags do not contain any edges or if they contain all edges.
     */
    const LBitset<LEdge> edge { exclusiveZone().edge() };

    /*
     * If either the width or height of size() is 0, it indicates that the client expects the
     * compositor to assign it. In such cases, or if the surface is anchored to both opposite edges,
     * all edges or no edge, we assign it the entire available space of its exclusive zone.
     * The client can choose a different size if desired. For more details see the LExclusiveZone doc.
     */

    if (newSize.w() == 0 || edge == 0 || anchor().checkAll(LEdgeLeft | LEdgeRight))
        newSize.setW(exclusiveZone().rect().w());

    if (newSize.h() == 0 || edge == 0 || anchor().checkAll(LEdgeTop | LEdgeBottom))
        newSize.setH(exclusiveZone().rect().h());

    configureSize(newSize);
}
//! [configureRequest]
