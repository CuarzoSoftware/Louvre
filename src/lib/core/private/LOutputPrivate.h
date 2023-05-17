#ifndef LOUTPUTPRIVATE_H
#define LOUTPUTPRIVATE_H

#include <LOutput.h>

class Louvre::LOutput::LOutputPrivate
{

public:
    LOutputPrivate() = default;
    ~LOutputPrivate() = default;

    LOutputPrivate(const LOutputPrivate&) = delete;
    LOutputPrivate& operator= (const LOutputPrivate&) = delete;

    LOutput *output;
    LRect rectC;
    LOutputMode *pendingMode = nullptr;

    // Painter
    LPainter *painter = nullptr;

    State state = Uninitialized;

    void *graphicBackendData = nullptr;

    // Setup Methods
    bool initialize(LCompositor *compositor);
    void globalScaleChanged(Int32 oldScale, Int32 newScale);

    // Compositor
    LCompositor *compositor = nullptr;
    wl_global *global = nullptr;

    // Params
    Int32 outputScale = 1;

    bool scheduledRepaint = false;
    UInt64 presentationSeq = 0;
    timespec presentationTime;

    // Called by the backend
    void backendInitialized();
    void backendBeforePaint();
    void backendAfterPaint();
    void backendPageFlipped();
};

#endif // LOUTPUTPRIVATE_H
