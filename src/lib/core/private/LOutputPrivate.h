#ifndef LOUTPUTPRIVATE_H
#define LOUTPUTPRIVATE_H

#include <LOutput.h>

LPRIVATE_CLASS(LOutput)
    LOutput *output;
    LRect rectC;
    LOutputMode *pendingMode = nullptr;
    std::thread::id threadId;

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

    timespec presentationTime;

    // Called by the backend
    void backendInitializeGL();
    void backendPaintGL();
    void backendResizeGL();
    void backendUninitializeGL();
    void backendPageFlipped();
};

#endif // LOUTPUTPRIVATE_H
