#include <LLayerRole.h>
#include <LCursor.h>
#include <LSurface.h>
#include <LCompositor.h>
#include <LSeat.h>
#include <LKeyboard.h>

using namespace Louvre;

//! [rolePos]
SkIPoint LLayerRole::rolePos() const
{
    // If no output has been assigned, use the user-defined position.
    if (!exclusiveOutput())
        return surface()->pos();

    if (anchor() == (LEdgeTop | LEdgeLeft))
    {
        m_rolePos = exclusiveOutput()->pos() + exclusiveZone().rect().topLeft();
    }
    else if (anchor() == (LEdgeTop | LEdgeRight))
    {
        m_rolePos.fX = exclusiveOutput()->pos().x() + exclusiveZone().rect().x()
                       + exclusiveZone().rect().width() - surface()->size().width();
        m_rolePos.fY = exclusiveOutput()->pos().y() + exclusiveZone().rect().y();
    }
    else if (anchor() == (LEdgeBottom | LEdgeRight))
    {
        m_rolePos.fX = exclusiveOutput()->pos().x() + exclusiveZone().rect().x()
                       + exclusiveZone().rect().width() - surface()->size().width();
        m_rolePos.fY = exclusiveOutput()->pos().y() + exclusiveZone().rect().y()
                       + exclusiveZone().rect().height() - surface()->size().height();
    }
    else if (anchor() == (LEdgeBottom | LEdgeLeft))
    {
        m_rolePos.fX = exclusiveOutput()->pos().x() + exclusiveZone().rect().x();
        m_rolePos.fY = exclusiveOutput()->pos().y() + exclusiveZone().rect().y()
                       + exclusiveZone().rect().height() - surface()->size().height();
    }
    else if (anchor() == LEdgeTop || anchor() == (LEdgeLeft | LEdgeTop | LEdgeRight))
    {
        m_rolePos.fX = exclusiveOutput()->pos().x() + exclusiveZone().rect().x()
                       + (exclusiveZone().rect().width() - surface()->size().width()) / 2;
        m_rolePos.fY = exclusiveOutput()->pos().y() + exclusiveZone().rect().y();
    }
    else if (anchor() == LEdgeBottom || anchor() == (LEdgeLeft | LEdgeBottom | LEdgeRight))
    {
        m_rolePos.fX = exclusiveOutput()->pos().x() + exclusiveZone().rect().x()
                       + (exclusiveZone().rect().width() - surface()->size().width()) / 2;
        m_rolePos.fY = exclusiveOutput()->pos().y() + exclusiveZone().rect().y()
                       + exclusiveZone().rect().height() - surface()->size().height();
    }
    else if (anchor() == LEdgeLeft || anchor() == (LEdgeBottom | LEdgeLeft | LEdgeTop))
    {
        m_rolePos.fX = exclusiveOutput()->pos().x() + exclusiveZone().rect().x();
        m_rolePos.fY = exclusiveOutput()->pos().y() + exclusiveZone().rect().y()
                       + (exclusiveZone().rect().height() - surface()->size().height()) / 2;
    }
    else if (anchor() == LEdgeRight || anchor() == (LEdgeBottom | LEdgeRight | LEdgeTop))
    {
        m_rolePos.fX = exclusiveOutput()->pos().x() + exclusiveZone().rect().x()
                       + exclusiveZone().rect().width() - surface()->size().width();
        m_rolePos.fY = exclusiveOutput()->pos().y() + exclusiveZone().rect().y()
                       + (exclusiveZone().rect().height() - surface()->size().height()) / 2;
    }
    else
        m_rolePos = exclusiveOutput()->pos() +
                    exclusiveZone().rect().topLeft() +
                    SkIPoint::Make(
                        (exclusiveZone().rect().size().width() - surface()->size().width()) / 2,
                        (exclusiveZone().rect().size().height() - surface()->size().height()) / 2);

    // Add extra margins.
    if (anchor().has(LEdgeTop))
        m_rolePos.fY = m_rolePos.y() + margins().top;
    if (anchor().has(LEdgeBottom))
        m_rolePos.fY = m_rolePos.y() - margins().bottom;
    if (anchor().has(LEdgeLeft))
        m_rolePos.fX = m_rolePos.x() + margins().left;
    if (anchor().has(LEdgeRight))
        m_rolePos.fX = m_rolePos.x() - margins().right;

    return m_rolePos;
}
//! [rolePos]

//! [atomsChanged]
void LLayerRole::atomsChanged(CZBitset<AtomChanges> changes, const Atoms &prevAtoms)
{
    L_UNUSED(prevAtoms);

    if (changes.has(KeyboardInteractivityChanged))
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

    if (changes.has(MarginsChanged))
        surface()->requestNextFrame(false);

    if (exclusiveOutput() && changes.has(
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
    SkISize newSize { size() };

    /*
     * LExclusiveZone calculates the single edge the surface is anchored to.
     * It can also return 0 if the anchor() flags do not contain any edges or if they contain all edges.
     */
    const CZBitset<LEdge> edge { exclusiveZone().edge() };

    /*
     * If either the width or height of size() is 0, it indicates that the client expects the
     * compositor to assign it. In such cases, or if the surface is anchored to both opposite edges,
     * all edges or no edge, we assign it the entire available space of its exclusive zone.
     * The client can choose a different size if desired. For more details see the LExclusiveZone doc.
     */

    if (newSize.width() == 0 || edge == 0 || anchor().hasAll(LEdgeLeft | LEdgeRight))
        newSize.fWidth = exclusiveZone().rect().width();

    if (newSize.height() == 0 || edge == 0 || anchor().hasAll(LEdgeTop | LEdgeBottom))
        newSize.fHeight = exclusiveZone().rect().height();

    configureSize(newSize);
}
//! [configureRequest]
