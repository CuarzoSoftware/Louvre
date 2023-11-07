#ifndef SURFACE_H
#define SURFACE_H

#include <LSurface.h>
#include <map>

using namespace Louvre;
using namespace std;

class Surface : public LSurface
{
public:
    Surface(LSurface::Params *params);
    void mappingChanged() override;
    void orderChanged() override;
    void minimizedChanged() override;

    // Temp variables used in paintGL()
    bool firstMap = true;
    bool occluded = false;
    bool isRenderable = false;
    LRect currentRectC;
    LRegion currentOpaqueTransposedC;
    LRegion currentDamageTransposedC;
    LRegion currentOpaqueTransposedCSum;
    LRegion currentTraslucentTransposedC;

    // Cached data for each output
    struct OutputData
    {
        UInt32 lastRenderedDamageId;
        LRect previousRectC;
        bool changedOrder = true;
    };
    map<LOutput*,OutputData>outputsMap;

    // Handle to prevent looking in the map each time
    OutputData *currentOutputData;
};

#endif // SURFACE_H
