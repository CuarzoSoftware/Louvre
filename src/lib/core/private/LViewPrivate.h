#ifndef LVIEWPRIVATE_H
#define LVIEWPRIVATE_H

#include <LRegion.h>
#include <LView.h>
#include <LRect.h>
#include <map>

using namespace Louvre;

LPRIVATE_CLASS(LView)

    LScene *scene = nullptr;
    bool clippingEnabled = false;
    bool visible = true;
    bool inputEnabled = false;
    bool scalingEnabled = false;
    bool cacheScalingEnabled;
    LSize scaledSizeC;
    LView *parent = nullptr;
    std::list<LView*>::iterator parentLink;
    std::list<LView*>children;
    UInt32 type;
    Float32 opacity = 1.f;
    Float32 cachedMultipliedOpacity;
    void *backendData = nullptr;
    LPointF axisScalig;

    /* TMP paintGL() vars */

    // If completly occluded by other views
    bool occluded = false;

    // Cache of mapped() call
    bool currentMapped = false;

    // Cache scale() == Global Scale
    bool bufferScaleMatchGlobalScale = false;

    // Cache rect (posC(), sizeC())
    LRect currentRectC = LRect(-1,-1,-1,-1);

    // Cached data for each output
    struct ViewOutputData
    {
        Float32 prevMultipliedOpacity = 1.f;
        UInt32 lastRenderedDamageId;
        LRect previousRectC;
        bool changedOrder = true;
        bool prevMapped = false;
        LRegion prevParentClippingC;
    };

    std::map<LOutput*,ViewOutputData>outputsMap;

    // Handle to prevent looking in the map each time
    ViewOutputData *currentOutputData;

    LRegion currentOpaqueTransposedC;
    LRegion currentDamageTransposedC;
    LRegion currentOpaqueTransposedCSum;
    LRegion currentTraslucentTransposedC;
    LRegion childrenOpaqueTransposedCSum;
};

#endif // LVIEWPRIVATE_H
