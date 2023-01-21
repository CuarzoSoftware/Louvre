#ifndef TOPLEVELROLE_H
#define TOPLEVELROLE_H

#include <LToplevelRole.h>

using namespace Louvre;

class ToplevelRole : public LToplevelRole
{
public:
    ToplevelRole(Params *params);

    void setMaximizedRequest() override;
    void maximizedChanged() override;
    void fullscreenChanged() override;
    void startMoveRequest() override;
    void startResizeRequest(ResizeEdge edge) override;


};

#endif // TOPLEVELROLE_H
