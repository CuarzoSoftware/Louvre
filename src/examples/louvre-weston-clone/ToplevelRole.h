#ifndef TOPLEVELROLE_H
#define TOPLEVELROLE_H

#include <LToplevelRole.h>

using namespace Louvre;

class ToplevelRole : public LToplevelRole
{
public:
    ToplevelRole(void *params);

    void configureRequest() override;
    void setMaximizedRequest() override;
    void setFullscreenRequest(LOutput *output) override;
    void unsetFullscreenRequest() override;
    void maximizedChanged() override;
    void fullscreenChanged() override;
    void startMoveRequest() override;
    void startResizeRequest(ResizeEdge edge) override;

    LRect rectBeforeFullscreen;
    StateFlags statesBeforeFullscreen;
};

#endif // TOPLEVELROLE_H
