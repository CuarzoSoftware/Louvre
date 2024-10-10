#ifndef LAYERROLE_H
#define LAYERROLE_H

#include <LLayerRole.h>

using namespace Louvre;

class LayerRole final : public LLayerRole
{
public:
    LayerRole(const void *params) noexcept;
    void configureRequest() override;

    bool isShelf { false };
};

#endif // LAYERROLE_H
