#ifndef TOPLEVELROLE_H
#define TOPLEVELROLE_H

#include <LToplevelRole.h>

using namespace Louvre;

class ToplevelRole final : public LToplevelRole
{
public:
    ToplevelRole(const void *params) noexcept;

    void configurationChanged(LBitset<ConfigurationChanges> changes) override;
    void configureRequest() noexcept override;
    void setMaximizedRequest() noexcept override;
    void setFullscreenRequest(LOutput *output) noexcept override;
    void unsetFullscreenRequest() noexcept override;
    void startMoveRequest(const LEvent &triggeringEvent) noexcept override;
    void startResizeRequest(const LEvent &triggeringEvent, ResizeEdge edge) noexcept override;

    LRect rectBeforeFullscreen;
    LBitset<State> statesBeforeFullscreen;
};

#endif // TOPLEVELROLE_H
