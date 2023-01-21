#ifndef SURFACE_H
#define SURFACE_H

#include <LSurface.h>
#include <unordered_map>

using namespace Louvre;
using namespace std;

class Surface : public LSurface
{
public:
    Surface(LSurface::Params *params, GLuint textureUnit = 1);

    void repaint();

    void mappingChanged() override;
    void raised() override;
    UInt64 allOutputsRequestedNewFrame();

    // Indica si es la primera vez que se mapea desde su creación (utilizado en Surface::mappingChanged() para posicionarla centrada en pantalla)
    bool firstMap = true;

    // Parámetros de la superficie por salida
    struct OutputParams
    {
        // Almacena daños de la superficie en cada buffer de la salida
        LRegion damagesG[2];

        // Almacena si cambió en cada buffer de la salida
        bool changed[2];

        // Almacena la recta al renderizar en un buffer de la salida
        LRect rectC[2];

        // Almacena si cambió de orden en un buffer de la salida
        bool changedOrder[2];

        // Daños totales
        LRegion totalDamagesG;

        LRegion totalTranslucentDamagesG;
        LRegion totalOpaqueDamagesG;

        LRegion temporalOpaqueRegionG;
        LRegion temporalTranslucentRegionG;

        // Indica si la salida actual desea invocar Surface::requestNextFrame
        bool requestedNewFrame = false;
        bool bufferScaleMatchGlobalScale = false;
    };

    unordered_map<LOutput*,OutputParams>outputParams;

    // Variable temporal para no tener que buscar los parametros en el hash params reiteradas veces
    OutputParams *cp;
};

#endif // SURFACE_H
