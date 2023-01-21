#ifndef OUTPUT_H
#define OUTPUT_H

#include <LOutput.h>
#include <LRegion.h>

using namespace Louvre;

class Output : public LOutput
{
public:
    Output();

    void addExposedRect(const LRect &rect);

    void initializeGL() override;
    void paintGL() override;

    // Si es true vuelve a repintar toda la salida en el prox frame
    bool fullRefresh = true;

    // Regiones expuestas del FRONT y BACK buffer
    LRegion exposedRegionG[2];

    // Almacena recta del cursor (si no es posible composición por hardware)
    LRect cursorRectG[2];

    // Indica si es la primera vez que se renderiza luego de la inicialización
    bool first[2];
};

#endif // OUTPUT_H
