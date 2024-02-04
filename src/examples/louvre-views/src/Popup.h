#ifndef POPUP_H
#define POPUP_H

#include <LPopupRole.h>

using namespace Louvre;

class Popup : public LPopupRole
{
public:
    Popup(void *params);
    void configureRequest() override;
};

#endif // POPUP_H
