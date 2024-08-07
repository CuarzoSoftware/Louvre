#ifndef LBACKENDPRIVATE_H
#define LBACKENDPRIVATE_H

#include <LNamespaces.h>
#include <LContentType.h>
#include <vector>

namespace Louvre
{
    struct LGraphicBackendInterface
    {
        UInt32                              (*backendGetId)();
        void *                              (*backendGetContextHandle)();
        bool                                (*backendInitialize)();
        void                                (*backendUninitialize)();
        void                                (*backendSuspend)();
        void                                (*backendResume)();
        const std::vector<LOutput*>*        (*backendGetConnectedOutputs)();
        UInt32                              (*backendGetRendererGPUs)();
        const std::vector<LDMAFormat>*      (*backendGetDMAFormats)();
        const std::vector<LDMAFormat>*      (*backendGetScanoutDMAFormats)();
        EGLDisplay                          (*backendGetAllocatorEGLDisplay)();
        EGLContext                          (*backendGetAllocatorEGLContext)();
        dev_t                               (*backendGetAllocatorDeviceId)();

        /* TEXTURES */
        bool                                (*textureCreateFromCPUBuffer)(LTexture *texture, const LSize &size, UInt32 stride, UInt32 format, const void *pixels);
        bool                                (*textureCreateFromWaylandDRM)(LTexture *texture, void *wlBuffer);
        bool                                (*textureCreateFromDMA)(LTexture *texture, const LDMAPlanes *planes);
        bool                                (*textureUpdateRect)(LTexture *texture, UInt32 stride, const LRect &dst, const void *pixels);
        UInt32                              (*textureGetID)(LOutput *output, LTexture *texture);
        GLenum                              (*textureGetTarget)(LTexture *texture);
        void                                (*textureDestroy)(LTexture *texture);

        /* OUTPUT */
        bool                                (*outputInitialize)(LOutput *output);
        bool                                (*outputRepaint)(LOutput *output);
        void                                (*outputUninitialize)(LOutput *output);
        bool                                (*outputHasBufferDamageSupport)(LOutput *output);
        void                                (*outputSetBufferDamage)(LOutput *output, LRegion &region);

        /* OUTPUT PROPS */
        const char *                        (*outputGetName)(LOutput *output);
        const char *                        (*outputGetManufacturerName)(LOutput *output);
        const char *                        (*outputGetModelName)(LOutput *output);
        const char *                        (*outputGetDescription)(LOutput *output);
        const LSize *                       (*outputGetPhysicalSize)(LOutput *output);
        Int32                               (*outputGetSubPixel)(LOutput *output);

        /* OUTPUT BUFFERING */
        Int32                               (*outputGetCurrentBufferIndex)(LOutput *output);
        UInt32                              (*outputGetBuffersCount)(LOutput *output);
        LTexture *                          (*outputGetBuffer)(LOutput *output, UInt32 bufferIndex);

        /* OUTPUT GAMMA */
        UInt32                              (*outputGetGammaSize)(LOutput *output);
        bool                                (*outputSetGamma)(LOutput *output, const LGammaTable &gamma);

        /* OUTPUT V-SYNC */
        bool                                (*outputHasVSyncControlSupport)(LOutput *output);
        bool                                (*outputIsVSyncEnabled)(LOutput *output);
        bool                                (*outputEnableVSync)(LOutput *output, bool enabled);
        void                                (*outputSetRefreshRateLimit)(LOutput *output, Int32 hz);
        Int32                               (*outputGetRefreshRateLimit)(LOutput *output);

        /* OUTPUT TIME */
        clockid_t                           (*outputGetClock)(LOutput *output);

        /* OUTPUT CURSOR */
        bool                                (*outputHasHardwareCursorSupport)(LOutput *output);
        void                                (*outputSetCursorTexture)(LOutput *output, UChar8 *buffer);
        void                                (*outputSetCursorPosition)(LOutput *output, const LPoint &position);

        /* OUTPUT MODES */
        const LOutputMode *                 (*outputGetPreferredMode)(LOutput *output);
        const LOutputMode *                 (*outputGetCurrentMode)(LOutput *output);
        const std::vector<LOutputMode*> *   (*outputGetModes)(LOutput *output);
        bool                                (*outputSetMode)(LOutput *output, LOutputMode *mode);

        /* OUTPUT CONTENT TYPE */

        // Must return LContentTypeNone by default
        LContentType                        (*outputGetContentType)(LOutput *output);

        // The assigned type must always later be the same value returned by outputGetContentType()
        void                                (*outputSetContentType)(LOutput *output, LContentType type);

        /* DIRECT SCANOUT */
        bool                                (*outputSetScanoutBuffer)(LOutput *output, LTexture *texture);
    };

    struct LInputBackendInterface
    {
        UInt32                             (*backendGetId)();
        void *                             (*backendGetContextHandle)();
        const std::vector<LInputDevice*> * (*backendGetDevices)();
        bool                               (*backendInitialize)();
        void                               (*backendUninitialize)();
        void                               (*backendSuspend)();
        void                               (*backendResume)();
        void                               (*backendSetLeds)(UInt32);
        void                               (*backendForceUpdate)();
    };
};

#endif // LBACKENDPRIVATE_H
