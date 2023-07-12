#ifndef LLAYERVIEWPRIVATE_H
#define LLAYERVIEWPRIVATE_H

#include <LLayerView.h>
#include <LRegion.h>

using namespace Louvre;

LPRIVATE_CLASS(LLayerView)
    LPoint nativePos;
    LSize nativeSize = LSize(256, 256);
    LRegion *inputRegion = nullptr;
    LRegion dummyRegion;
    std::list<LOutput *>outputs;

    LPoint tmpPos;
};

#endif // LLAYERVIEWPRIVATE_H
