#ifndef LGRAPHICBACKEND
#define LGRAPHICBACKEND

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <CZ/Louvre/LNamespaces.h>
#include <CZ/Louvre/LContentType.h>
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
    static const std::vector<LDMAFormat>*   backendGetDMAFormats();
    static const std::vector<LDMAFormat>*   backendGetScanoutDMAFormats();
    static EGLDisplay                       backendGetAllocatorEGLDisplay();
    static EGLContext                       backendGetAllocatorEGLContext();
    static LGPU*                            backendGetAllocatorDevice();
    static const std::vector<LGPU*> *       backendGetDevices();

    /* TEXTURES */
    static bool                             textureCreateFromCPUBuffer(LTexture *texture, SkISize size, UInt32 stride, UInt32 format, const void *pixels);
    static bool                             textureCreateFromWaylandDRM(LTexture *texture,void *wlBuffer);
    static bool                             textureCreateFromDMA(LTexture *texture, const LDMAPlanes *planes);
    static bool                             textureCreateFromGL(LTexture *texture, GLuint id, GLenum target, UInt32 format, SkISize size, bool transferOwnership);
    static bool                             textureUpdateRect(LTexture *texture, UInt32 stride, const SkIRect &dst, const void *pixels);
    static bool                             textureWriteBegin(LTexture *texture);
    static bool                             textureWriteUpdate(LTexture *texture, UInt32 stride, const SkIRect &dst, const void *pixels);
    static bool                             textureWriteEnd(LTexture *texture);
    static UInt32                           textureGetID(LOutput *output, LTexture *texture);
    static GLenum                           textureGetTarget(LTexture *texture);
    static void                             textureSetFence(LTexture *texture);
    static void                             textureDestroy(LTexture *texture);

    /* OUTPUT */
    static bool                             outputInitialize(LOutput *output);
    static bool                             outputRepaint(LOutput *output);
    static void                             outputUninitialize(LOutput *output);
    static bool                             outputHasBufferDamageSupport(LOutput *output);
    static void                             outputSetBufferDamage(LOutput *output, SkRegion &region);

    /* OUTPUT PROPS */
    static const char *                     outputGetName(LOutput *output);
    static const char *                     outputGetManufacturerName(LOutput *output);
    static const char *                     outputGetModelName(LOutput *output);
    static const char *                     outputGetDescription(LOutput *output);
    static const char *                     outputGetSerial(LOutput *output);
    static SkISize                          outputGetPhysicalSize(LOutput *output);
    static Int32                            outputGetSubPixel(LOutput *output);
    static LGPU *                           outputGetDevice(LOutput *output);
    static UInt32                           outputGetID(LOutput *output);
    static bool                             outputIsNonDesktop(LOutput *output);

    /* OUTPUT BUFFERING */
    static UInt32                           outputGetFramebufferID(LOutput *output);
    static Int32                            outputGetCurrentBufferIndex(LOutput *output);
    static UInt32                           outputGetBuffersCount(LOutput *output);
    static UInt32                           outputGetCurrentBufferAge(LOutput *output);
    static LTexture *                       outputGetBuffer(LOutput *output, UInt32 bufferIndex);
    static void                             outputDestroyBuffers(std::vector<LTexture*> &textures);
    static void                             outputLockCurrentBuffer(LOutput *output, bool locked);

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
    static void                             outputSetCursorTexture(LOutput *output, UInt8 *buffer);
    static void                             outputSetCursorPosition(LOutput *output, SkIPoint position);

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

    /* DRM LEASE */
    static int                              backendCreateLease(const std::vector<LOutput*> &outputs);
    static void                             backendRevokeLease(int fd);

    /* DRM Backend only */
    static int                              openRestricted(const char *path, int flags, void *userData);
    static void                             closeRestricted(int fd, void *userData);
};

#endif // LGRAPHICBACKEND
