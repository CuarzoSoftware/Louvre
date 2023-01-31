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
    void setCompositor(LCompositor *compositor);
    void initialize();
    void globalScaleChanged(Int32 oldScale, Int32 newScale);

    // Paint
    static void startRenderLoop(void *data);
    static void renderLoop(void *data);

    // Compositor
    LCompositor *compositor = nullptr;
    wl_global *global = nullptr;

    // Params
    Int32 outputScale = 1;

    // Render thread
    eventfd_t renderValue = 1;
    std::thread *renderThread;
    pollfd renderPoll;
    bool scheduledRepaint = false;
    UInt64 presentationSeq = 0;



};

#endif // LOUTPUTPRIVATE_H
