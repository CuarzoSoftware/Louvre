#ifndef LVIEWPRIVATE_H
#define LVIEWPRIVATE_H

#include <LRegion.h>
#include <LView.h>
#include <LRect.h>
#include <map>
#include <thread>
#include <LPainter.h>
#include <GL/gl.h>

using namespace Louvre;

LPRIVATE_CLASS(LView)

    void removeThread(Louvre::LView *view, thread::id thread);

    UInt32 type;
    LScene *scene = nullptr;
    LView *parent = nullptr;
    std::list<LView*>children;
    std::list<LView*>::iterator parentLink;
    std::list<LView*>::iterator compositorLink;
    bool repaintCalled = false;
    GLenum sFactor = GL_SRC_ALPHA;
    GLenum dFactor = GL_ONE_MINUS_SRC_ALPHA;

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
    struct ViewThreadData
    {
        LOutput *o = nullptr;
        Float32 prevOpacity = 1.f;
        UInt32 lastRenderedDamageId = 0;
        LRect prevRect;
        LRect prevLocalRect;
        bool changedOrder = true;
        bool prevMapped = false;
        LRegion prevParentClipping;
    };

    struct ViewCache
    {
        ViewThreadData *voD;
        LRect rect;
        LRect localRect;
        LRegion damage;
        LRegion translucent;
        LRegion opaque;
        LRegion opaqueOverlay;
        Float32 opacity;
        LSizeF scalingVector;
        bool mapped = false;
        bool occluded = false;
        bool scalingEnabled;
    } cache;

    std::map<std::thread::id,ViewThreadData>threadsMap;

    // Input related
    bool pointerIsOver = false;

    void markAsChangedOrder(bool includeChildren = true);
    void damageScene(LSceneView *s);
};

#endif // LVIEWPRIVATE_H
