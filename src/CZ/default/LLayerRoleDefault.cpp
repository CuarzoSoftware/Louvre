#include <CZ/Louvre/Roles/LLayerRole.h>
#include <CZ/Louvre/Cursor/LCursor.h>
#include <CZ/Louvre/Roles/LSurface.h>
#include <CZ/Louvre/LCompositor.h>
#include <CZ/Louvre/Seat/LSeat.h>
#include <CZ/Louvre/Seat/LKeyboard.h>

using namespace CZ;

//! [rolePos]
SkIPoint LLayerRole::rolePos() const
{
    // If no output has been assigned, use the user-defined position.
    if (!exclusiveOutput())
        return surface()->pos();

    if (anchor() == (CZEdgeTop | CZEdgeLeft))
    {
        m_rolePos = exclusiveOutput()->pos() + exclusiveZone().rect().topLeft();
    }
    else if (anchor() == (CZEdgeTop | CZEdgeRight))
    {
        m_rolePos.fX = exclusiveOutput()->pos().x() + exclusiveZone().rect().x()
                       + exclusiveZone().rect().width() - surface()->size().width();
        m_rolePos.fY = exclusiveOutput()->pos().y() + exclusiveZone().rect().y();
    }
    else if (anchor() == (CZEdgeBottom | CZEdgeRight))
    {
        m_rolePos.fX = exclusiveOutput()->pos().x() + exclusiveZone().rect().x()
                       + exclusiveZone().rect().width() - surface()->size().width();
        m_rolePos.fY = exclusiveOutput()->pos().y() + exclusiveZone().rect().y()
                       + exclusiveZone().rect().height() - surface()->size().height();
    }
    else if (anchor() == (CZEdgeBottom | CZEdgeLeft))
    {
        m_rolePos.fX = exclusiveOutput()->pos().x() + exclusiveZone().rect().x();
        m_rolePos.fY = exclusiveOutput()->pos().y() + exclusiveZone().rect().y()
                       + exclusiveZone().rect().height() - surface()->size().height();
    }
    else if (anchor() == CZEdgeTop || anchor() == (CZEdgeLeft | CZEdgeTop | CZEdgeRight))
    {
        m_rolePos.fX = exclusiveOutput()->pos().x() + exclusiveZone().rect().x()
                       + (exclusiveZone().rect().width() - surface()->size().width()) / 2;
        m_rolePos.fY = exclusiveOutput()->pos().y() + exclusiveZone().rect().y();
    }
    else if (anchor() == CZEdgeBottom || anchor() == (CZEdgeLeft | CZEdgeBottom | CZEdgeRight))
    {
        m_rolePos.fX = exclusiveOutput()->pos().x() + exclusiveZone().rect().x()
                       + (exclusiveZone().rect().width() - surface()->size().width()) / 2;
        m_rolePos.fY = exclusiveOutput()->pos().y() + exclusiveZone().rect().y()
                       + exclusiveZone().rect().height() - surface()->size().height();
    }
    else if (anchor() == CZEdgeLeft || anchor() == (CZEdgeBottom | CZEdgeLeft | CZEdgeTop))
    {
        m_rolePos.fX = exclusiveOutput()->pos().x() + exclusiveZone().rect().x();
        m_rolePos.fY = exclusiveOutput()->pos().y() + exclusiveZone().rect().y()
                       + (exclusiveZone().rect().height() - surface()->size().height()) / 2;
    }
    else if (anchor() == CZEdgeRight || anchor() == (CZEdgeBottom | CZEdgeRight | CZEdgeTop))
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
    if (anchor().has(CZEdgeTop))
        m_rolePos.fY = m_rolePos.y() + margins().top;
    if (anchor().has(CZEdgeBottom))
        m_rolePos.fY = m_rolePos.y() - margins().bottom;
    if (anchor().has(CZEdgeLeft))
        m_rolePos.fX = m_rolePos.x() + margins().left;
    if (anchor().has(CZEdgeRight))
        m_rolePos.fX = m_rolePos.x() - margins().right;

    return m_rolePos;
}
//! [rolePos]

//! [atomsChanged]
void LLayerRole::stateChanged(CZBitset<Changes> changes, const State &prev) noexcept
{
    CZ_UNUSED(prev);

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
    const CZBitset<CZEdge> edge { exclusiveZone().edge() };

    /*
     * If either the width or height of size() is 0, it indicates that the client expects the
     * compositor to assign it. In such cases, or if the surface is anchored to both opposite edges,
     * all edges or no edge, we assign it the entire available space of its exclusive zone.
     * The client can choose a different size if desired. For more details see the LExclusiveZone doc.
     */

    if (newSize.width() == 0 || edge == 0 || anchor().hasAll(CZEdgeLeft | CZEdgeRight))
        newSize.fWidth = exclusiveZone().rect().width();

    if (newSize.height() == 0 || edge == 0 || anchor().hasAll(CZEdgeTop | CZEdgeBottom))
        newSize.fHeight = exclusiveZone().rect().height();

    configureSize(newSize);
}
//! [configureRequest]
