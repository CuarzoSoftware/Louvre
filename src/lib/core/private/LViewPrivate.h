#ifndef LVIEWPRIVATE_H
#define LVIEWPRIVATE_H

#include <LRegion.h>
#include <LView.h>
#include <LRect.h>
#include <map>

using namespace Louvre;

LPRIVATE_CLASS(LView)

    struct ViewInterface
    {
        void (*unref)(const LView *view);
        bool (*mapped)(const LView *view);

        LTexture *(*texture)(const LView *view);
        void (*setTexture)(const LView *view, LTexture *texture);

        const LRegion &(*damageC)(const LView *view);
        void (*setDamageC)(const LView *view, const LRegion &region);

        const LRegion &(*inputRegionC)(const LView *view);
        void (*setInputRegionC)(const LView *view, const LRegion &region);

        const LRegion &(*opaqueRegionC)(const LView *view);
        void (*setOpaqueRegionC)(const LView *view, const LRegion &region);

        const LRegion &(*translucentRegionC)(const LView *view);
        void (*setTranslucentRegionC)(const LView *view, const LRegion &region);

        const LPoint &(*posC)(const LView *view);
        void (*setPosC)(const LView *view, const LPoint &pos);

        const LPoint &(*sizeC)(const LView *view);
        void (*setSizeC)(const LView *view, const LSize &size);

        Int32 (*scale)(const LView *view);
    } interface;

    LScene *scene = nullptr;
    bool clippingEnabled = false;
    bool visible = true;
    bool inputEnabled = false;
    bool scalingEnabled = false;
    LSize scaledSizeC;
    LView *parent = nullptr;
    std::list<LView*>::iterator parentLink;
    std::list<LView*>children;
    UInt32 type;
    Float32 opacity = 1.f;
    void *backendData = nullptr;

    /* TMP paintGL() vars */

    // If completly occluded by other views
    bool occluded = false;

    // Cache of mapped() call
    bool currentMapped = false;

    // Cache scale() == Global Scale
    bool bufferScaleMatchGlobalScale = false;

    // Cache rect (posC(), sizeC())
    LRect currentRectC;

    // Cached data for each output
    struct ViewOutputData
    {
        UInt32 lastRenderedDamageId;
        LRect previousRectC;
        bool changedOrder = true;
        bool prevMapped = false;
    };

    std::map<LOutput*,ViewOutputData>outputsMap;

    // Handle to prevent looking in the map each time
    ViewOutputData *currentOutputData;

    LRegion currentOpaqueTransposedC;
    LRegion currentDamageTransposedC;
    LRegion currentOpaqueTransposedCSum;
    LRegion currentTraslucentTransposedC;
};

#endif // LVIEWPRIVATE_H
