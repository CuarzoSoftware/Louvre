#ifndef POPUP_H
#define POPUP_H

#include <LPopupRole.h>

using namespace Louvre;

class Popup final : public LPopupRole
{
public:
    Popup(const void *params) noexcept;

    void configureRequest() override;
};

#endif // POPUP_H
