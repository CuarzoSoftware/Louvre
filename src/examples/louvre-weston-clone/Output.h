#ifndef OUTPUT_H
#define OUTPUT_H

#include <LOutput.h>
#include <LRegion.h>

using namespace Louvre;

class Output : public LOutput
{
public:
    Output();

    void fullDamage();
    void initializeGL() override;
    void resizeGL() override;
    void paintGL() override;

    LRegion damage, newDamage;

    // Output rect since the last paintGL()
    LRect lastRectC;

    // Almacena recta del cursor (si no es posible composición por hardware)
    LRect cursorRectG[2];
};

#endif // OUTPUT_H
