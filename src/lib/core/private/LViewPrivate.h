#ifndef LVIEWPRIVATE_H
#define LVIEWPRIVATE_H

#include <LRegion.h>
#include <LView.h>
#include <LRect.h>
#include <map>
#include <LPainter.h>

using namespace Louvre;

LPRIVATE_CLASS(LView)

    void removeOutput(Louvre::LView *view, LOutput *output);

    UInt32 type;
    LScene *scene = nullptr;
    LView *parent = nullptr;
    std::list<LView*>children;
    std::list<LView*>::iterator parentLink;
    std::list<LView*>::iterator compositorLink;

    Float32 opacity = 1.f;
    LSizeF scalingVector = LSizeF(1.f, 1.f);

    bool visible = true;
    bool inputEnabled = false;
    bool scalingEnabled = false;
    bool parentScalingEnabled = false;
    bool parentOffsetEnabled = true;
    bool parentClippingEnabled = false;
    bool parentOpacityEnabled = true;
    bool forceRequestNextFrameEnabled = false;

    LPoint tmpPos;
    LSize tmpSize;
    LSizeF tmpScalingVector;

    // Cached data for each output
    struct ViewOutputData
    {
        Float32 prevOpacity = 1.f;
        UInt32 lastRenderedDamageId;
        LRect prevRect;
        bool changedOrder = true;
        bool prevMapped = false;
        LRegion prevParentClipping;
    };

    struct ViewCache
    {
        ViewOutputData *voD;
        LRect rect;
        LRegion damage;
        LRegion translucent;
        LRegion opaque;
        LRegion opaqueOverlay;
        Float32 opacity;
        LSizeF scalingVector;
        bool mapped = false;
        bool occluded = false;
        bool bufferScaleMatchGlobalScale = false;
        bool scalingEnabled;
    } cache;

    std::map<LOutput*,ViewOutputData>outputsMap;

    LPainterMask *masks[LPAINTER_MAX_MASKS];
};

#endif // LVIEWPRIVATE_H
