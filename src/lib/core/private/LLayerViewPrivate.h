#ifndef LLAYERVIEWPRIVATE_H
#define LLAYERVIEWPRIVATE_H

#include <LLayerView.h>
#include <LRegion.h>

using namespace Louvre;

LPRIVATE_CLASS(LLayerView)
    LPoint customPosC;
    LSize customSizeC;
    LRegion inputRegionC;
    LPoint tmpPosC;
    std::list<LOutput *>outputs;
};

#endif // LLAYERVIEWPRIVATE_H
