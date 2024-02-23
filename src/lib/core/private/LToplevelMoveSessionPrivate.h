#ifndef LTOPLEVELMOVESESSIONPRIVATE_H
#define LTOPLEVELMOVESESSIONPRIVATE_H

#include <LToplevelMoveSession.h>

class Louvre::LToplevelMoveSession::Factory
{
public:

    using UniqueMoveSession = std::unique_ptr<LToplevelMoveSession, Factory>;

    void operator()(LToplevelMoveSession* ptr) const
    {
        delete ptr;
    }

    static inline UniqueMoveSession makeUnique(LToplevelRole *toplevel, const LEvent &triggeringEvent, const LPoint &initDragPos, const LBox &bounds)
    {
        return UniqueMoveSession(new LToplevelMoveSession(toplevel, triggeringEvent, initDragPos, bounds), Factory());
    }
};

#endif // LTOPLEVELMOVESESSIONPRIVATE_H
