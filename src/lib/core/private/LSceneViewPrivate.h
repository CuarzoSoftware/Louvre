#ifndef LSCENEVIEWPRIVATE_H
#define LSCENEVIEWPRIVATE_H

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

        bool foundRenderableSaledView;

        // Only for non LScene
        LRegion translucentTransposedSum;
    };

    LRGBAF clearColor = {0,0,0,0};
    std::map<std::thread::id, ThreadData> threadsMap;

    // Quck handle to current output data
    ThreadData *currentThreadData;

    void clearTmpVariables(ThreadData *oD);
    void damageAll(ThreadData *oD);
    void checkRectChange(ThreadData *oD);
    void cachePass(LView *view, ThreadData *oD);
    void calcNewDamage(LView *view, ThreadData *oD);
    void drawOpaqueDamage(LView *view, ThreadData *oD);
    void drawBackground(ThreadData *oD, bool addToOpaqueSum);
    void drawTranslucentDamage(LView *view, ThreadData *oD);

    void parentClipping(LView *parent, LRegion *region);
};

#endif // LSCENEVIEWPRIVATE_H
