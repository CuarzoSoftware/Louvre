#ifndef LTOPLEVELRESIZESESSION_H
#define LTOPLEVELRESIZESESSION_H

#include <LToplevelRole.h>

class Louvre::LToplevelResizeSession : public LObject
{
public:
    void setResizePointPos(const LPoint &pos);
    void stop();
    LToplevelRole *toplevel() const;
    const LEvent &triggeringEvent() const;

private:
    friend class LToplevelRole;
    LToplevelResizeSession();
    ~LToplevelResizeSession();
    void handleGeometryChange();
    LToplevelRole *m_toplevel;
    LPoint m_initPos;
    LSize m_initSize;
    LSize m_minSize;
    LPoint m_initResizePointPos;
    LPoint m_currentResizePointPos;
    LToplevelRole::ResizeEdge m_edge;
    LBox m_bounds;
    LEvent *m_triggeringEvent { nullptr };
    bool m_stopped { false };
};

#endif // LTOPLEVELRESIZESESSION_H
