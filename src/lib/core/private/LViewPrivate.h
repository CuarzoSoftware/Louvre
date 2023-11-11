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

    enum LViewState : UInt32
    {
        /* Sometimes view ordering can change while a scene is emitting an event,
         * in such cases it must start again. These flags are for preventing re-sending
         * the event to the same view */
        PointerMoveDone         = 1 << 0,
        PointerButtonDone       = 1 << 2,
        PointerAxisDone         = 1 << 3,
        KeyModifiersDone        = 1 << 4,
        KeyDone                 = 1 << 5,
    };

    UInt32 state = 0;

    void removeThread(Louvre::LView *view, std::thread::id thread);

    UInt32 type;
    LScene *scene = nullptr;
    LView *parent = nullptr;
    std::list<LView*>children;
    std::list<LView*>::iterator parentLink;
    std::list<LView*>::iterator compositorLink;
    bool repaintCalled = false;
    GLenum sFactor = GL_SRC_ALPHA;
    GLenum dFactor = GL_ONE_MINUS_SRC_ALPHA;
    LRGBAF colorFactor = {1.f, 1.f, 1.f, 1.f};
    bool colorFactorEnabled = false;

    Float32 opacity = 1.f;
    LSizeF scalingVector = LSizeF(1.f, 1.f);

    bool visible = true;
    bool inputEnabled = false;
    bool scalingEnabled = false;
    bool parentScalingEnabled = false;
    bool parentOffsetEnabled = true;
    bool clippingEnabled = false;
    bool parentClippingEnabled = false;
    bool parentOpacityEnabled = true;
    bool forceRequestNextFrameEnabled = false;

    LRect clippingRect;

    LPoint tmpPos;
    LSize tmpSize;
    LSizeF tmpScalingVector;

    // This is used for detecting changes on a view since the last time it was drawn on a specific output
    struct ViewThreadData
    {
        LOutput *o = nullptr;
        Float32 prevOpacity = 1.f;
        UInt32 lastRenderedDamageId = 0;
        LRect prevRect;
        LRect prevLocalRect;
        bool changedOrder = true;
        bool prevMapped = false;
        LRegion prevClipping;
        LRGBAF prevColorFactor;
        bool prevColorFactorEnabled = false;
    };

    // This is used to prevent invoking heavy methods
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
        bool isFullyTrans;
    };

    ViewCache cache;

    std::map<std::thread::id,ViewThreadData>threadsMap;

    // Input related
    bool pointerIsOver = false;
    bool blockPointerEnabled = true;

    void markAsChangedOrder(bool includeChildren = true);
    void damageScene(LSceneView *s);

    inline static void removeFlagWithChildren(LView *view, UInt32 flag)
    {
        view->imp()->state &= ~flag;

        for (LView *child : view->imp()->children)
            removeFlagWithChildren(child, flag);
    }
};

#endif // LVIEWPRIVATE_H
