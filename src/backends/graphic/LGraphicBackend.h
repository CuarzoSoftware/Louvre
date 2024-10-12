#ifndef LGRAPHICBACKEND
#define LGRAPHICBACKEND

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <LNamespaces.h>
#include <LContentType.h>
#include <vector>

class Louvre::LGraphicBackend
{
public:
    static UInt32                           backendGetId();
    static void *                           backendGetContextHandle();
    static bool                             backendInitialize();
    static void                             backendUninitialize();
    static void                             backendSuspend();
    static void                             backendResume();
    static const std::vector<LOutput*>*     backendGetConnectedOutputs();
    static UInt32                           backendGetRendererGPUs();
    static const std::vector<LDMAFormat>*   backendGetDMAFormats();
    static const std::vector<LDMAFormat>*   backendGetScanoutDMAFormats();
    static EGLDisplay                       backendGetAllocatorEGLDisplay();
    static EGLContext                       backendGetAllocatorEGLContext();
    static dev_t                            backendGetAllocatorDeviceId();

    /* TEXTURES */
    static bool                             textureCreateFromCPUBuffer(LTexture *texture, const LSize &size, UInt32 stride, UInt32 format, const void *pixels);
    static bool                             textureCreateFromWaylandDRM(LTexture *texture,void *wlBuffer);
    static bool                             textureCreateFromDMA(LTexture *texture, const LDMAPlanes *planes);
    static bool                             textureUpdateRect(LTexture *texture, UInt32 stride, const LRect &dst, const void *pixels);
    static UInt32                           textureGetID(LOutput *output, LTexture *texture);
    static GLenum                           textureGetTarget(LTexture *texture);
    static void                             textureDestroy(LTexture *texture);

    /* OUTPUT */
    static bool                             outputInitialize(LOutput *output);
    static bool                             outputRepaint(LOutput *output);
    static void                             outputUninitialize(LOutput *output);
    static bool                             outputHasBufferDamageSupport(LOutput *output);
    static void                             outputSetBufferDamage(LOutput *output, LRegion &region);

    /* OUTPUT PROPS */
    static const char *                     outputGetName(LOutput *output);
    static const char *                     outputGetManufacturerName(LOutput *output);
    static const char *                     outputGetModelName(LOutput *output);
    static const char *                     outputGetDescription(LOutput *output);
    static const LSize *                    outputGetPhysicalSize(LOutput *output);
    static Int32                            outputGetSubPixel(LOutput *output);

    /* OUTPUT BUFFERING */
    static UInt32                           outputGetFramebufferID(LOutput *output);
    static Int32                            outputGetCurrentBufferIndex(LOutput *output);
    static UInt32                           outputGetBuffersCount(LOutput *output);
    static LTexture *                       outputGetBuffer(LOutput *output, UInt32 bufferIndex);
    static void                             outputDestroyBuffers(std::vector<LTexture*> &textures);

    /* OUTPUT GAMMA */
    static UInt32                           outputGetGammaSize(LOutput *output);
    static bool                             outputSetGamma(LOutput *output, const LGammaTable &table);

    /* OUTPUT V-SYNC */
    static bool                             outputHasVSyncControlSupport(LOutput *output);
    static bool                             outputIsVSyncEnabled(LOutput *output);
    static bool                             outputEnableVSync(LOutput *output, bool enabled);
    static void                             outputSetRefreshRateLimit(LOutput *output, Int32 hz);
    static Int32                            outputGetRefreshRateLimit(LOutput *output);

    /* OUTPUT TIME */
    static clockid_t                        outputGetClock(LOutput *output);

    /* OUTPUT CURSOR */
    static bool                             outputHasHardwareCursorSupport(LOutput *output);
    static void                             outputSetCursorTexture(LOutput *output, UChar8 *buffer);
    static void                             outputSetCursorPosition(LOutput *output, const LPoint &position);

    /* OUTPUT MODES */
    static const LOutputMode *              outputGetPreferredMode(LOutput *output);
    static const LOutputMode *              outputGetCurrentMode(LOutput *output);
    static const std::vector<LOutputMode*>* outputGetModes(LOutput *output);
    static bool                             outputSetMode(LOutput *output, LOutputMode *mode);

    /* OUTPUT CONTENT TYPE */
    static LContentType                     outputGetContentType(LOutput *output);
    static void                             outputSetContentType(LOutput *output, LContentType type);

    /* DIRECT SCANOUT */
    static bool                             outputSetScanoutBuffer(LOutput *output, LTexture *texture);
};

#endif // LGRAPHICBACKEND
