#ifndef LSCENEVIEWPRIVATE_H
#define LSCENEVIEWPRIVATE_H

#include <LSceneView.h>
#include <LRegion.h>
#include <map>

using namespace Louvre;

LPRIVATE_CLASS(LSceneView)
    LPoint customPos;

    LFramebuffer *fb = nullptr;
    std::list<LOutput*>outputs;
    LRegion input;

    struct OutputData
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
    std::map<LOutput*, OutputData> outputsMap;

    // Quck handle to current output data
    OutputData *currentOutputData;

    void clearTmpVariables(OutputData *oD);
    void damageAll(OutputData *oD);
    void checkRectChange(OutputData *oD);
    void cachePass(LView *view, OutputData *oD);
    void calcNewDamage(LView *view, OutputData *oD);
    void drawOpaqueDamage(LView *view, OutputData *oD);
    void drawBackground(OutputData *oD, bool addToOpaqueSum);
    void drawTranslucentDamage(LView *view, OutputData *oD);

    void parentClipping(LView *parent, LRegion *region);
};

#endif // LSCENEVIEWPRIVATE_H
