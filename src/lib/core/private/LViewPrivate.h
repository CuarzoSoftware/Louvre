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

        RepaintCalled           = 1 << 6,
        ColorFactor             = 1 << 7,
        Visible                 = 1 << 8,
        Input                   = 1 << 9,
        Scaling                 = 1 << 10,
        ParentScaling           = 1 << 11,
        ParentOffset            = 1 << 12,
        Clipping                = 1 << 13,
        ParentClipping          = 1 << 14,
        ParentOpacity           = 1 << 15,
        ForceRequestNextFrame   = 1 << 16,
        PointerIsOver           = 1 << 17,
        BlockPointer            = 1 << 18,
        AutoBlendFunc           = 1 << 19
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

    UInt32 state { Visible | ParentOffset | ParentOpacity | BlockPointer | AutoBlendFunc };
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
    std::list<LView*>::iterator parentLink;

    void removeThread(Louvre::LView *view, std::thread::id thread);
    void markAsChangedOrder(bool includeChildren = true);
    void damageScene(LSceneView *s);

    inline void removeFlag(UInt32 flag)
    {
        state &= ~flag;
    }

    inline void addFlag(UInt32 flag)
    {
        state |= flag;
    }

    inline bool hasFlag(UInt32 flag)
    {
        return state & flag;
    }

    inline void setFlag(UInt32 flag, bool enable)
    {
        if (enable)
            addFlag(flag);
        else
            removeFlag(flag);
    }

    inline static void removeFlagWithChildren(LView *view, UInt32 flag)
    {
        view->imp()->removeFlag(flag);

        for (LView *child : view->imp()->children)
            removeFlagWithChildren(child, flag);
    }
};

#endif // LVIEWPRIVATE_H
