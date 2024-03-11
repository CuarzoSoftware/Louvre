#ifndef LVIEWPRIVATE_H
#define LVIEWPRIVATE_H

#include <LRegion.h>
#include <LView.h>
#include <LRect.h>
#include <LPainter.h>
#include <GL/gl.h>
#include <thread>
#include <map>

using namespace Louvre;

LPRIVATE_CLASS(LView)

    enum LViewState : UInt64
    {
        IsScene                 = 1UL << 0,

        PointerEvents           = 1UL << 1,
        KeyboardEvents          = 1UL << 2,
        TouchEvents             = 1UL << 3,

        BlockPointer            = 1UL << 4,
        BlockTouch              = 1UL << 5,

        RepaintCalled           = 1UL << 6,
        ColorFactor             = 1UL << 7,
        Visible                 = 1UL << 8,
        Scaling                 = 1UL << 9,
        ParentScaling           = 1UL << 10,
        ParentOffset            = 1UL << 11,
        Clipping                = 1UL << 12,
        ParentClipping          = 1UL << 13,
        ParentOpacity           = 1UL << 14,
        ForceRequestNextFrame   = 1UL << 15,
        AutoBlendFunc           = 1UL << 16,

        PointerIsOver           = 1UL << 17,

        PendingSwipeEnd         = 1UL << 18,
        PendingPinchEnd         = 1UL << 19,
        PendingHoldEnd          = 1UL << 20,

        PointerMoveDone         = 1UL << 21,
        PointerButtonDone       = 1UL << 22,
        PointerScrollDone       = 1UL << 23,
        PointerSwipeBeginDone   = 1UL << 24,
        PointerSwipeUpdateDone  = 1UL << 25,
        PointerSwipeEndDone     = 1UL << 26,
        PointerPinchBeginDone   = 1UL << 27,
        PointerPinchUpdateDone  = 1UL << 28,
        PointerPinchEndDone     = 1UL << 28,
        PointerHoldBeginDone    = 1UL << 30,
        PointerHoldEndDone      = 1UL << 31,
        KeyDone                 = 1UL << 32,
        TouchDownDone           = 1UL << 33,
        TouchMoveDone           = 1UL << 34,
        TouchUpDone             = 1UL << 35,
        TouchFrameDone          = 1UL << 36,
        TouchCancelDone         = 1UL << 37,
    };

    // This is used for detecting changes on a view since the last time it was drawn on a specific output
    struct ViewThreadData
    {
        LOutput *o { nullptr };
        Float32 prevOpacity { 1.f };
        UInt32 lastRenderedDamageId { 0 };
        LRect prevRect;
        LRect prevLocalRect;
        bool changedOrder { true };
        bool prevMapped { false };
        LRegion prevClipping;
        LRGBAF prevColorFactor;
        bool prevColorFactorEnabled { false };
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
        bool mapped { false };
        bool occluded { false };
        bool scalingEnabled;
        bool isFullyTrans;
    };

    LView *view;
    UInt64 state { Visible | ParentOffset | ParentOpacity | BlockPointer | AutoBlendFunc };
    ViewCache cache;

    UInt32 type;
    LView *parent { nullptr };
    std::list<LView*>children;

    GLenum sRGBFactor    { GL_SRC_ALPHA };
    GLenum dRGBFactor    { GL_ONE_MINUS_SRC_ALPHA };
    GLenum sAlphaFactor  { GL_SRC_ALPHA };
    GLenum dAlphaFactor  { GL_ONE_MINUS_SRC_ALPHA };

    LRGBAF colorFactor {1.f, 1.f, 1.f, 1.f};
    Float32 opacity { 1.f };
    LSizeF scalingVector { 1.f, 1.f };
    LRect clippingRect;
    LPoint tmpPoint;
    LSize tmpSize;
    LPointF tmpPointF;

    std::map<std::thread::id,ViewThreadData>threadsMap;
    LScene *scene { nullptr };
    LScene *currentScene { nullptr };
    std::list<LView*>::iterator parentLink;

    void removeThread(Louvre::LView *view, std::thread::id thread);
    void markAsChangedOrder(bool includeChildren = true);
    void damageScene(LSceneView *s);
    void sceneChanged(LScene *newScene);

    inline void removeFlag(UInt64 flag)
    {
        state &= ~flag;
    }

    inline void addFlag(UInt64 flag)
    {
        state |= flag;
    }

    inline bool hasFlag(UInt64 flag)
    {
        return state & flag;
    }

    inline void setFlag(UInt64 flag, bool enable)
    {
        if (enable)
            addFlag(flag);
        else
            removeFlag(flag);
    }

    inline static void removeFlagWithChildren(LView *view, UInt64 flag)
    {
        view->imp()->removeFlag(flag);

        for (LView *child : view->imp()->children)
            removeFlagWithChildren(child, flag);
    }
};

#endif // LVIEWPRIVATE_H
