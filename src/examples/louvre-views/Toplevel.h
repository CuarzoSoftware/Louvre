#ifndef TOPLEVEL_H
#define TOPLEVEL_H

#include <LToplevelRole.h>

class Toplevel : public LToplevelRole
{
public:
    Toplevel(Params *params);

    void setMaximizedRequest() override;
    void maximizedChanged() override;
};

#endif // TOPLEVEL_H
