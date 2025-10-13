#include <CZ/Core/Events/CZTouchFrameEvent.h>
#include <CZ/Louvre/Roles/LSurface.h>
#include <CZ/Louvre/Seat/LTouch.h>
#include <CZ/Louvre/Seat/LTouchPoint.h>
#include <CZ/Louvre/Seat/LOutput.h>
#include <CZ/Louvre/Seat/LSeat.h>
#include <CZ/Louvre/Seat/LPointer.h>
#include <CZ/Louvre/LCompositor.h>
#include <CZ/Louvre/Cursor/LCursor.h>
#include <CZ/Louvre/LLog.h>
#include <CZ/Louvre/Roles/LToplevelRole.h>
#include <CZ/Louvre/Events/LSurfaceCommitEvent.h>

using namespace CZ;

//! [pointerConstraintModeChanged]
void LSurface::pointerConstraintModeChanged()
{
    /* No default implementation */
}
//! [pointerConstraintModeChanged]

//! [pointerConstraintRegionChanged]
void LSurface::pointerConstraintRegionChanged()
{
    /* No default implementation */
}
//! [pointerConstraintRegionChanged]

//! [lockedPointerPosHintChanged]
void LSurface::lockedPointerPosHintChanged()
{
    /* No default implementation */
}
//! [lockedPointerPosHintChanged]

//! [damageChanged]
void LSurface::damageChanged()
{
    repaintOutputs();
}
//! [damageChanged]

//! [roleChanged]
void LSurface::roleChanged() noexcept
{
    repaintOutputs();
}
//! [roleChanged]

//! [mappingChanged]
void LSurface::mappingChanged()
{
    compositor()->repaintAllOutputs();

    if (!mapped())
    {
        if (hasPointerFocus())
            seat()->pointer()->setFocus(nullptr);

        if (hasKeyboardFocus())
            seat()->pointer()->setFocus(nullptr);

        bool hadTouchPoint { false };

        CZTouchUpEvent e {};

        for (auto *tp : seat()->touch()->touchPoints())
        {
            if (tp->surface() == this)
            {
                e.id = tp->id();
                tp->sendUpEvent(e);
                hadTouchPoint = true;
            }
        }

        if (hadTouchPoint)
            seat()->touch()->sendFrameEvent({});
    }

    LOutput *activeOutput { cursor()->output() };

    if (!activeOutput)
        return;

    /* If the surface is a toplevel, we place it at the center of the screen */
    if (mapped() && toplevel())
    {
        const SkISize size { toplevel()->windowGeometry().size() };
        const SkIPoint availGeoPos { activeOutput->pos() + activeOutput->availableGeometry().topLeft() };

        setPos(
            availGeoPos.x() + (activeOutput->availableGeometry().width() - size.width()) / 2,
            availGeoPos.y() + (activeOutput->availableGeometry().height() - size.height()) / 2);

        if (pos().y() < availGeoPos.y())
            setY(availGeoPos.y());
    }
}
//! [mappingChanged]

//! [bufferScaleChanged]
void LSurface::bufferScaleChanged()
{
    repaintOutputs();
}
//! [bufferScaleChanged]

//! [bufferTransformChanged]
void LSurface::bufferTransformChanged()
{
    repaintOutputs();
}
//! [bufferTransformChanged]

//! [bufferSizeChanged]
void LSurface::bufferSizeChanged()
{
    repaintOutputs();
}
//! [bufferSizeChanged]

//! [sizeChanged]
void LSurface::sizeChanged()
{
    repaintOutputs();
}
//! [sizeChanged]

//! [srcRectChanged]
void LSurface::srcRectChanged()
{
    repaintOutputs();
}
//! [srcRectChanged]

//! [opaqueRegionChanged]
void LSurface::opaqueRegionChanged()
{
    repaintOutputs();
}
//! [opaqueRegionChanged]

//! [invisibleRegionChanged]
void LSurface::invisibleRegionChanged()
{
    repaintOutputs();
}
//! [invisibleRegionChanged]

//! [inputRegionChanged]
void LSurface::inputRegionChanged()
{
    /* No default implementation */
}
//! [inputRegionChanged]

//! [requestedRepaint]
void LSurface::requestedRepaint()
{
    repaintOutputs();
}
//! [requestedRepaint]

//! [prefersVSyncChanged]
void LSurface::prefersVSyncChanged()
{
    /* No default implementation */
}
//! [prefersVSyncChanged]

//! [contentTypeChanged]
void LSurface::contentTypeChanged()
{
    /* No default implementation */
}
//! [contentTypeChanged]

void LSurface::commitEvent(const LSurfaceCommitEvent &e) noexcept
{
    using Changes = LSurfaceCommitEvent::Changes;

    if (e.changes.has(Changes::BufferSizeChanged))
        bufferSizeChanged();

    if (e.changes.has(Changes::SizeChanged))
        sizeChanged();

    if (e.changes.has(Changes::SrcRectChanged))
        srcRectChanged();

    if (e.changes.has(Changes::BufferScaleChanged))
        bufferScaleChanged();

    if (e.changes.has(Changes::BufferTransformChanged))
        bufferTransformChanged();

    if (e.changes.has(Changes::DamageRegionChanged))
        damageChanged();

    if (e.changes.has(Changes::InputRegionChanged))
        inputRegionChanged();

    if (e.changes.has(Changes::PointerConstraintModeChanged))
        pointerConstraintModeChanged();

    if (e.changes.has(Changes::PointerConstraintRegionChanged))
        pointerConstraintRegionChanged();

    if (e.changes.has(Changes::LockedPointerPosHintChanged))
        lockedPointerPosHintChanged();

    if (e.changes.has(Changes::OpaqueRegionChanged))
        opaqueRegionChanged();

    if (e.changes.has(Changes::InvisibleRegionChanged))
        invisibleRegionChanged();

    if (e.changes.has(Changes::VSyncChanged))
        prefersVSyncChanged();

    if (e.changes.has(Changes::ContentTypeChanged))
        contentTypeChanged();

    if (e.changes.has(Changes::MappingChanged))
        mappingChanged();
}

bool LSurface::event(const CZEvent &e) noexcept
{
    if (e.typeIsAnyOf(CZEvent::Type::LSurfaceCommit))
        commitEvent((const LSurfaceCommitEvent&)e);

    return LFactoryObject::event(e);
}
