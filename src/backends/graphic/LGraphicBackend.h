#ifndef LGRAPHICBACKEND
#define LGRAPHICBACKEND

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <LNamespaces.h>
#include <list>

using namespace std;

class Louvre::LGraphicBackend
{
public:

    static bool initialize(LCompositor *compositor);
    static void uninitialize(LCompositor *compositor);
    static const list<LOutput*>*getConnectedOutputs(LCompositor *compositor);

    /* OUTPUTS */

    static bool initializeOutput(LOutput *output);
    static bool scheduleOutputRepaint(LOutput *output);
    static void uninitializeOutput(LOutput *output);

    /* Connector physical size in mm */
    static const LSize *getOutputPhysicalSize(LOutput *output);
    static Int32 getOutputCurrentBufferIndex(LOutput *output);
    static const char *getOutputName(LOutput *output);
    static const char *getOutputManufacturerName(LOutput *output);
    static const char *getOutputModelName(LOutput *output);
    static const char *getOutputDescription(LOutput *output);
    static const LOutputMode *getOutputPreferredMode(LOutput *output);
    static const LOutputMode *getOutputCurrentMode(LOutput *output);
    static const list<LOutputMode*> *getOutputModes(LOutput *output);
    static bool setOutputMode(LOutput *output, LOutputMode *mode);

    /* OUTPUT MODES */

    static const LSize *getOutputModeSize(LOutputMode *mode);
    static Int32 getOutputModeRefreshRate(LOutputMode *mode);
    static bool getOutputModeIsPreferred(LOutputMode *mode);

    /* CURSOR */

    static bool hasHardwareCursorSupport(LOutput *output);
    static void setCursorTexture(LOutput *output, UChar8 *buffer);
    static void setCursorPosition(LOutput *output, const LPoint &position);

    /* BUFFERS */

    static const list<LDMAFormat *> *getDMAFormats(LCompositor *compositor);
    static EGLDisplay getAllocatorEGLDisplay(LCompositor *compositor);
    static EGLContext getAllocatorEGLContext(LCompositor *compositor);

    static bool createTextureFromCPUBuffer(LTexture *texture,
                                           const LSize &size,
                                           UInt32 stride,
                                           UInt32 format,
                                           const void *pixels);

    static bool createTextureFromWaylandDRM(LTexture *texture,
                                            void *wlBuffer);

    static bool createTextureFromDMA(LTexture *texture,
                                     const LDMAPlanes *planes);

    static bool updateTextureRect(LTexture *texture,
                                  UInt32 stride,
                                  const LRect &dst,
                                  const void *pixels);

    static UInt32 getTextureID(LOutput *output, LTexture *texture);

    static void destroyTexture(LTexture *texture);
};


#endif // LGRAPHICBACKEND
