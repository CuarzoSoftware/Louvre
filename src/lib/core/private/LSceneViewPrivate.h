#ifndef LSCENEVIEWPRIVATE_H
#define LSCENEVIEWPRIVATE_H

#include <LFramebuffer.h>
#include <LSceneView.h>
#include <LRegion.h>
#include <map>
#include <thread>

using namespace Louvre;

LPRIVATE_CLASS(LSceneView)
    LPoint customPos;

    LFramebuffer *fb = nullptr;
    std::list<LOutput*>outputs;
    LRegion input;

    struct ThreadData
    {
        // List of new damage calculated in prev frames
        std::list<LRegion*>prevDamageList;

        // New damage calculated on this frame
        LRegion newDamage;

        // Manually added damage
        LRegion manuallyAddedDamage;

        LRect prevRect;
        LCompositor *c;
        LPainter *p;
        LOutput *o = nullptr;
        Int32 n, w, h;
        LBox *boxes;
        LRegion opaqueTransposedSum;
        LRegion prevExternalExclude;

        // Only for non LScene
        LRegion translucentTransposedSum;
    };

    LRGBAF clearColor = {0,0,0,0};
    std::map<std::thread::id, ThreadData> threadsMap;

    // Quck handle to current output data
    ThreadData *currentThreadData;

    void calcNewDamage(LView *view);
    void drawOpaqueDamage(LView *view);
    void drawBackground(bool addToOpaqueSum);
    void drawTranslucentDamage(LView *view);

    void parentClipping(LView *parent, LRegion *region);

    inline void clearTmpVariables(ThreadData *oD)
    {
        oD->newDamage.clear();
        oD->opaqueTransposedSum.clear();
    }

    inline void damageAll(ThreadData *oD)
    {
        oD->newDamage.clear();
        oD->newDamage.addRect(fb->rect());
        oD->newDamage.addRect(fb->rect());
    }

    inline void checkRectChange(ThreadData *oD)
    {
        if (oD->prevRect.size() != fb->rect().size())
        {
            damageAll(oD);
            oD->prevRect.setSize(fb->rect().size());
        }
    }
};

#endif // LSCENEVIEWPRIVATE_H
