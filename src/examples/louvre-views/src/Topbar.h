#ifndef TOPBAR_H
#define TOPBAR_H

#include <LLayerView.h>
#include <LSolidColorView.h>
#include "UITextureView.h"

class Output;

using namespace Louvre;

class Topbar final : public LLayerView
{
public:
    Topbar(Output *output);
    ~Topbar();

    void update();
    void updateOutputInfo();

    Output *output;

    // White bar
    LSolidColorView background;

    // Louvre logo
    UITextureView logo;

    // Clock text
    LTextureView clock;

    // Output mode text
    LTextureView outputInfo;

    // Oversampling indicator
    LTextureView oversamplingLabel;

    // V-Sync indicator
    LTextureView vSyncLabel;

    // Current app title
    LTextureView appName;

    void pointerEnterEvent(const LPointerEnterEvent &) override;
    void pointerMoveEvent(const LPointerMoveEvent &) override;
    bool nativeMapped() const noexcept override;
};

#endif // TOPBAR_H
