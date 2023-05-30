#ifndef LOUTPUTPRIVATE_H
#define LOUTPUTPRIVATE_H

#include <LOutput.h>

LPRIVATE_CLASS(LOutput)
    LOutput *output;
    LRect rectC;
    LOutputMode *pendingMode = nullptr;

    // Painter
    LPainter *painter = nullptr;

    State state = Uninitialized;

    void *graphicBackendData = nullptr;

    // Setup Methods
    bool initialize();
    void globalScaleChanged(Int32 oldScale, Int32 newScale);

    // Compositor
    wl_global *global = nullptr;

    // Params
    Int32 outputScale = 1;

    UInt64 presentationSeq = 0;
    timespec presentationTime;

    // Called by the backend
    void backendInitialized();
    void backendBeforePaint();
    void backendAfterPaint();
    void backendPageFlipped();
};

#endif // LOUTPUTPRIVATE_H
