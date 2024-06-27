#ifndef TOPLEVELROLE_H
#define TOPLEVELROLE_H

#include <LToplevelRole.h>

using namespace Louvre;

class ToplevelRole final : public LToplevelRole
{
public:
    using LToplevelRole::LToplevelRole;

    void atomsChanged(LBitset<AtomChanges> changes, const Atoms &prev) override;
    void configureRequest() noexcept override;
    void setMaximizedRequest() noexcept override;
    void setFullscreenRequest(LOutput *output) noexcept override;
    void unsetFullscreenRequest() noexcept override;

    LRect rectBeforeFullscreen;
    LBitset<State> statesBeforeFullscreen;
};

#endif // TOPLEVELROLE_H
