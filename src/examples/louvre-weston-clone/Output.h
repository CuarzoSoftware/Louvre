#ifndef OUTPUT_H
#define OUTPUT_H

#include <LExclusiveZone.h>
#include <LOutput.h>
#include <LRegion.h>

using namespace Louvre;

class Output final : public LOutput
{
public:
    using LOutput::LOutput;

    std::unique_ptr<LTexture> backgroundTexture;
    LExclusiveZone topbarExclusiveZone {LEdgeTop, 32, this};
    LRect terminalIconRect;
    Float32 terminalIconAlpha { 1.0f };
    Float32 terminalIconAlphaPrev { 1.0f };

    void loadWallpaper() noexcept;
    void fullDamage()  noexcept;
    void initializeGL() noexcept override;
    void resizeGL() noexcept override;
    void moveGL() noexcept override;
    void paintGL() noexcept override;
    void setGammaRequest(LClient *client, const LGammaTable *gamma) noexcept override;

    // List of new damage calculated in prev frames
    bool damageListCreated = false;
    std::list<LRegion*>prevDamageList;

    // New damage calculated on this frame
    LRegion newDamage;

    // Output rect since the last paintGL()
    LRect lastRect;

    // Almacena recta del cursor (si no es posible composición por hardware)
    LRect cursorRect[2];

    LWeak<LSurface> fullscreenSurface;
    LRect dstClockRect;
    bool redrawClock { true };
};

#endif // OUTPUT_H
