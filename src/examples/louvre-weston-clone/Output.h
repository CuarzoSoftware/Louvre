#ifndef OUTPUT_H
#define OUTPUT_H

#include <LOutput.h>
#include <LRegion.h>

using namespace Louvre;

class Output : public LOutput
{
public:
    Output();

    LTexture *backgroundTexture = nullptr;

    Int32 topbarHeight;
    LTexture *terminalIconTexture = nullptr;
    LRect terminalIconRectC;
    Float32 terminalIconAlpha = 1.0f;
    Float32 terminalIconAlphaPrev = 1.0f;

    void fullDamage();
    void initializeGL() override;
    void resizeGL() override;
    void paintGL() override;

    // List of new damage calculated in prev frames
    bool damageListCreated = false;
    std::list<LRegion*>prevDamageList;

    // New damage calculated on this frame
    LRegion newDamage;

    // Output rect since the last paintGL()
    LRect lastRectC;

    // Almacena recta del cursor (si no es posible composición por hardware)
    LRect cursorRectG[2];

    LSurface *fullscreenSurface = nullptr;
    bool redrawClock = true;
    LRect dstClockRect;
};

#endif // OUTPUT_H
