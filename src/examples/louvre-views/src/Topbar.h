#ifndef TOPBAR_H
#define TOPBAR_H

#include <LSolidColorView.h>
#include <LExclusiveZone.h>
#include "Compositor.h"
#include "UITextureView.h"
#include "Global.h"

class Output;

using namespace Louvre;

class Topbar final : public LSolidColorView
{
public:
    Topbar(Output *output);
    ~Topbar();

    void update();
    void updateOutputInfo();

    LExclusiveZone exclusiveZone { LEdgeTop, TOPBAR_HEIGHT };

    Output *output;

    // Louvre logo
    UITextureView logo { G::Logo, this };

    // Clock text
    LTextureView clock { G::compositor()->clockTexture, this };

    // Output mode text
    LTextureView outputInfo { nullptr, this };

    // Oversampling indicator
    LTextureView oversamplingLabel { G::compositor()->oversamplingLabelTexture, this };

    // V-Sync indicator
    LTextureView vSyncLabel { G::compositor()->vSyncLabelTexture, this };

    // Current app title
    LTextureView appName { G::textures()->defaultTopbarAppName, this };

    void pointerEnterEvent(const LPointerEnterEvent &) override;
    void pointerMoveEvent(const LPointerMoveEvent &) override;
    bool nativeMapped() const noexcept override;
};

#endif // TOPBAR_H
