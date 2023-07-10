#ifndef LSCENEPRIVATE_H
#define LSCENEPRIVATE_H

#include <LLayerView.h>
#include <LScene.h>
#include <LRegion.h>
#include <map>

using namespace Louvre;

LPRIVATE_CLASS(LScene)
    struct OutputData
    {
        LRect prevRectC;
        LRegion prevDamageC;
        LRegion newDamageC;
        LCompositor *c;
        LPainter *p;
        LOutput *o = nullptr;
        Int32 n, w, h;
        LBox *boxes;
        LRegion opaqueTransposedCSum;
    };

    LRGBF clearColor = {0,0,0};
    std::map<LOutput*, OutputData>outputsMap;
    LLayerView mainView;

    void clearTmpVariables(OutputData &oD);
    void damageAll(OutputData &oD);
    void checkRectChange(OutputData &oD);
    void calcNewDamage(LView *view, OutputData &oD, bool force = false, LRegion *opacity = nullptr);
    void drawOpaqueDamage(LView *view, OutputData &oD, bool force = false);
    void drawBackground(OutputData &oD);
    void drawTranslucentDamage(LView *view, OutputData &oD, bool force = false, LRegion *opacity = nullptr);

    void parentClipping(LView *parent, LRegion *region);
    bool pointClippedByParent(LView *parent, const LPoint &point);
    LView *viewAtC(LView *view, const LPoint &pos);
};

#endif // LSCENEPRIVATE_H
