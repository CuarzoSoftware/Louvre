#ifndef LTOPLEVELRESIZESESSION_H
#define LTOPLEVELRESIZESESSION_H

#include <LToplevelRole.h>
#include <LTimer.h>

class Louvre::LToplevelResizeSession
{
public:
    // TODO: add doc

    /**
     * @brief Start an interactive toplevel resizing session.
     *
     * This method starts an interactive resizing session on a toplevel surface from one of its edges or corners.\n
     * You can restrict the space in which the surface expands by defining a rectangle given by the L, T, R, and B values.\n
     * If you do not want to restrict an edge, assign its value to LPointer::EdgeDisabled.
     *
     * To update the position and size of the Toplevel, call updateResizingToplevelSize() when the pointer moves and
     * updateResizingToplevelPos() when the toplevel size changes.\n
     * Once finished, call stopResizingToplevel() to end the session.
     *
     * @note The session will automatically cease if the toplevel is destroyed.
     *
     * @see See an example of its use in LToplevelRole::startResizeRequest().
     *
     * @param toplevel Toplevel that will change size.
     * @param edge Edge or corner from which the resizing will be performed.
     * @param globalDragPoint Current move point position (cursor position, touch point position, etc). TODO
     * @param minSize Minimum toplevel size.
     * @param L Restriction of the left edge.
     * @param T Restriction of the top edge.
     * @param R Restriction of the right edge.
     * @param B Restriction of the bottom edge.
     */
    bool start(const LEvent &triggeringEvent,
               LToplevelRole::ResizeEdge edge,
               const LPoint &initDragPoint,
               const LSize &minSize = LSize(0, 0),
               Int32 L = LToplevelRole::EdgeDisabled, Int32 T = LToplevelRole::EdgeDisabled,
               Int32 R = LToplevelRole::EdgeDisabled, Int32 B = LToplevelRole::EdgeDisabled);

    void updateDragPoint(const LPoint &pos);

    const std::vector<LToplevelResizeSession*>::const_iterator stop();

    inline LToplevelRole *toplevel() const
    {
        return m_toplevel;
    }

    inline const LEvent &triggeringEvent() const
    {
        return *m_triggeringEvent.get();
    }

    inline bool isActive() const
    {
        return m_isActive;
    }

private:
    friend class LToplevelRole;
    LToplevelResizeSession();
    ~LToplevelResizeSession();
    void handleGeometryChange();
    LToplevelRole *m_toplevel;
    LPoint m_initPos;
    LSize m_initSize;
    LSize m_minSize;
    LPoint m_initDragPoint;
    LPoint m_currentDragPoint;
    LToplevelRole::ResizeEdge m_edge;
    LBox m_bounds;
    std::unique_ptr<LEvent> m_triggeringEvent;
    UInt32 m_lastSerial { 0 };
    bool m_isActive { false };
    bool m_lastSerialHandled { true };
    LTimer m_ackTimer;
};

#endif // LTOPLEVELRESIZESESSION_H
