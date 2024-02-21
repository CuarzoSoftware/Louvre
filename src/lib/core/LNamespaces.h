#ifndef LNAMESPACES_H
#define LNAMESPACES_H

#include <protocols/Wayland/wayland.h>
#include <list>
#include <vector>
#include <memory>
#include <algorithm>
#include <string>

#define LOUVRE_MAX_SURFACE_SIZE 10000000
#define LOUVRE_GLOBAL_ITERS_BEFORE_DESTROY 5
#define LOUVRE_MAX_DMA_PLANES 4

// Globals
#define LOUVRE_WL_COMPOSITOR_VERSION 6
#define LOUVRE_WL_CALLBACK_VERSION 1
#define LOUVRE_WL_SEAT_VERSION 9
#define LOUVRE_WL_DATA_DEVICE_MANAGER_VERSION 3
#define LOUVRE_WL_OUTPUT_VERSION 4
#define LOUVRE_WL_SUBCOMPOSITOR_VERSION 1
#define LOUVRE_XDG_WM_BASE_VERSION 2
#define LOUVRE_XDG_DECORATION_MANAGER_VERSION 1
#define LOUVRE_WP_PRESENTATION_VERSION 1
#define LOUVRE_LINUX_DMA_BUF_VERSION 3
#define LOUVRE_VIEWPORTER_VERSION 1
#define LOUVRE_FRACTIONAL_SCALE_VERSION 1
#define LOUVRE_GAMMA_CONTROL_MANAGER_VERSION 1
#define LOUVRE_TEARING_CONTROL_MANAGER_VERSION 1
#define LOUVRE_RELATIVE_POINTER_MANAGER_VERSION 1
#define LOUVRE_POINTER_GESTURES_VERSION 3

#define L_UNUSED(object){(void)object;}

#define CAT(x, y) CAT_(x, y)
#define CAT_(x, y) x ## y

#define LPRIVATE_CLASS(class_name) \
    class class_name::CAT(class_name,Private){ \
        public: \
        CAT(class_name,Private)() = default; \
        ~CAT(class_name,Private)() = default; \
        CAT(class_name,Private)(const CAT(class_name,Private)&) = delete; \
        CAT(class_name,Private) &operator=(const CAT(class_name,Private)&) = delete;

#define LPRIVATE_CLASS_NO_COPY(class_name) \
class class_name::CAT(class_name,Private){ \
        public: \
        CAT(class_name,Private)(const CAT(class_name,Private)&) = delete; \
        CAT(class_name,Private) &operator=(const CAT(class_name,Private)&) = delete;

#define LPRIVATE_IMP(class_name) \
    class CAT(class_name,Private); \
    inline CAT(class_name,Private) *imp() const {return m_imp;}; \
    private: \
    CAT(class_name,Private) *m_imp = nullptr;

#define LPRIVATE_IMP_UNIQUE(class_name) \
    class CAT(class_name,Private); \
    inline CAT(class_name,Private) *imp() const {return m_imp.get();}; \
    private: \
        friend class std::unique_ptr<CAT(class_name,Private)>;\
        std::unique_ptr<CAT(class_name,Private)> m_imp;

#define LPRIVATE_INIT_UNIQUE(class_name) \
    m_imp(std::make_unique<CAT(class_name,Private)>())

template <typename T>
static inline void LVectorRemoveOne(std::vector<T>& vec, T val)
{
    auto it = std::find(vec.begin(), vec.end(), val);
    if (it != vec.end())
        vec.erase(it);
}

template <typename T>
static inline void LVectorRemoveAll(std::vector<T>& vec, T val)
{
    for (auto it = vec.begin(); it != vec.end();)
    {
        if (*it == val)
            it = vec.erase(it);
        else
            it++;
    }
}

template <typename T>
static inline void LVectorRemoveOneUnordered(std::vector<T>& vec, T val)
{
    auto it = std::find(vec.begin(), vec.end(), val);
    if (it != vec.end())
    {
        *it = std::move(vec.back());
        vec.pop_back();
    }
}

template <typename T>
static inline void LVectorPushBackIfNonexistent(std::vector<T>& vec, T val)
{
    auto it = std::find(vec.begin(), vec.end(), val);
    if (it == vec.end())
    {
        vec.push_back(val);
    }
}

template <typename T>
static inline void LVectorRemoveAllUnordered(std::vector<T>& vec, T val)
{
    for (auto it = vec.begin(); it != vec.end();)
    {
        retry:
        if (*it == val)
        {
            *it = std::move(vec.back());
            vec.pop_back();

            if (it != vec.end())
                goto retry;
        }
        else
            it++;
    }
}

/**
 * @namespace Louvre
 * @brief Namespaces
 * @page Namespaces
 */
namespace Louvre
{
    // API Classes
    class LObject;
    class LAnimation;
    class LGraphicBackend;
    class LInputBackend;
    class LClient;
    class LCompositor;
    class LOutput;
    class LOutputManager;
    class LOutputMode;
    class LOpenGL;
    class LPositioner;
    class LRegion;
    class LResource;
    class LSurface;
    class LTexture;

    // Painter
    class LPainter;
    class LRenderBuffer;
    class LFramebuffer;
    class LOutputFramebuffer;

    // Views
    class LScene;
    class LView;
    class LLayerView;
    class LSurfaceView;
    class LTextureView;
    class LSolidColorView;
    class LSceneView;
    class LSceneTouchPoint;

    // Data
    class LDataDevice;
    class LDataSource;
    class LDataOffer;
    class LDNDManager;

    // Surface roles
    class LBaseSurfaceRole;
    class LCursorRole;
    class LDNDIconRole;
    class LToplevelRole;
    class LPopupRole;
    class LSubsurfaceRole;

    // Input related
    class LSeat;
    class LPointer;
    class LKeyboard;
    class LTouch;
    class LCursor;
    class LXCursor;
    class LInputDevice;
    class LEvent;
    class LInputEvent;

    class LPointerEvent;
    class LPointerEnterEvent;
    class LPointerLeaveEvent;
    class LPointerMoveEvent;
    class LPointerButtonEvent;
    class LPointerScrollEvent;

    class LPointerSwipeBeginEvent;
    class LPointerSwipeUpdateEvent;
    class LPointerSwipeEndEvent;

    class LPointerPinchBeginEvent;
    class LPointerPinchUpdateEvent;
    class LPointerPinchEndEvent;

    class LPointerHoldBeginEvent;
    class LPointerHoldEndEvent;

    class LKeyboardEvent;
    class LKeyboardEnterEvent;
    class LKeyboardLeaveEvent;
    class LKeyboardKeyEvent;
    class LKeyboardModifiersEvent;

    class LTouchEvent;
    class LTouchDownEvent;
    class LTouchMoveEvent;
    class LTouchUpEvent;
    class LTouchFrameEvent;
    class LTouchCancelEvent;
    class LTouchPoint;

    class LToplevelResizeSession;
    class LToplevelMoveSession;

    // Other
    class LDMABuffer;

    // Utils
    class LLog;
    class LTime;
    class LTimer;
    class LLauncher;
    class LGammaTable;
    template <class TA, class TB> class LPointTemplate;
    template <class TA, class TB> class LRectTemplate;
    template <class T> class LBitset;

    // Types
    /// @brief 64 bits unsigned integer
    typedef uint64_t        UInt64;

    /// @brief 64 bits signed integer
    typedef int64_t         Int64;

    /// @brief 32 bits unsigned integer
    typedef uint32_t        UInt32;

    /// @brief 32 bits signed integer
    typedef int32_t         Int32;

    /// @brief 16 bits unsigned integer
    typedef uint16_t        UInt16;

    /// @brief 16 bits signed integer
    typedef int16_t         Int16;

    /// @brief 8 bits unsigned integer
    typedef uint8_t         UInt8;

    /// @brief 8 bits signed integer
    typedef int8_t          Int8;

    /// @brief 8 bits unsigned integer
    typedef unsigned char   UChar8;

    /// @brief 8 bits signed integer
    typedef char            Char8;

    /// @brief 64 bits float
    typedef double          Float64;

    /// @brief 32 bits float
    typedef float           Float32;

    /// @brief 24 bits Wayland float
    typedef wl_fixed_t      Float24;

    /// 2D vector of 32 bits integers
    using LPoint = LPointTemplate<Int32,Float32>;

    /// 2D vector of 32 bits integers
    using LSize = LPoint;

    /// 2D vector of 32 bits floats
    using LPointF = LPointTemplate<Float32,Int32>;

    /// 2D vector of 32 bits floats
    using LSizeF = LPointF;

    /// 4D vector of 32 bits integers
    using LRect = LRectTemplate<Int32,Float32>;

    /// 4D vector of 32 bits floats
    using LRectF = LRectTemplate<Float32,Int32>;

    typedef void* EGLContext;
    typedef void* EGLDisplay;
    typedef unsigned int GLuint;
    typedef unsigned int GLenum;

    /**
     * @brief Structure representing a 2D box.
     *
     * The LBox struct defines a 2D box using four integer coordinates (x1, y1, x2, y2).
     * It is typically used to represent bounding boxes or rectangular regions in 2D space.
     */
    struct LBox
    {
        /// The x-coordinate of the top-left corner of the box.
        Int32 x1;

        /// The y-coordinate of the top-left corner of the box.
        Int32 y1;

        /// The x-coordinate of the bottom-right corner of the box.
        Int32 x2;

        /// The y-coordinate of the bottom-right corner of the box.
        Int32 y2;
    };

    /**
     * @cond OMIT
     * @brief Structure representing DMA format and modifier.
     *
     * The LDMAFormat struct contains information about DMA format and modifier.
     * It is used to describe the format and memory layout of DMA planes used for texture generation.
     * Each LDMAFormat instance includes a format value and a modifier value.
     */
    struct LDMAFormat
    {
        /// The format of the DMA plane.
        UInt32 format;

        /// The modifier value specifying the memory layout.
        UInt64 modifier;
    };

    /// @endcond

    /**
     * @brief Direct Memory Access (DMA) planes.
     *
     * Use this struct to import DMA buffers with LTexture.
     */
    struct LDMAPlanes
    {
        /// Width of the buffer in pixels.
        UInt32 width;

        /// Height of the buffer in pixels.
        UInt32 height;

        /// DRM format of the buffer.
        UInt32 format;

        /// Number of file descriptors.
        UInt32 num_fds = 0;

        /// Array of file descriptors associated with each DMA plane.
        Int32 fds[LOUVRE_MAX_DMA_PLANES] = {-1};

        /// Array of strides for each DMA plane.
        UInt32 strides[LOUVRE_MAX_DMA_PLANES] = {0};

        /// Array of offsets for each DMA plane.
        UInt32 offsets[LOUVRE_MAX_DMA_PLANES] = {0};

        /// Array of modifiers for each DMA plane.
        UInt64 modifiers[LOUVRE_MAX_DMA_PLANES] = {0};
    };

    /**
     * @brief RGB color with floating-point components.
     *
     * The LRGBF struct defines an RGB color with three floating-point components (r, g, b).\n
     * Each component ranges from 0.0 to 1.0.
     */
    struct LRGBF
    {
        /// The red component of the RGB color (range: 0.0 to 1.0).
        Float32 r;

        /// The green component of the RGB color (range: 0.0 to 1.0).
        Float32 g;

        /// The blue component of the RGB color (range: 0.0 to 1.0).
        Float32 b;
    };

    /**
     * @brief RGBA color with floating-point components.
     *
     * The LRGBAF struct defines an RGBA color with four floating-point components (r, g, b, a).\n
     * Each component ranges from 0.0 to 1.0.
     */
    struct LRGBAF
    {
        /// The red component of the RGBA color (range: 0.0 to 1.0).
        Float32 r;

        /// The green component of the RGBA color (range: 0.0 to 1.0).
        Float32 g;

        /// The blue component of the RGBA color (range: 0.0 to 1.0).
        Float32 b;

        /// The alpha component of the RGBA color (range: 0.0 to 1.0).
        Float32 a;
    };

    /**
     * @brief ID values for the graphic backends shipped with Louvre.
     *
     * These IDs are returned by LSeat::graphicBackendId() to identify
     * the currently loaded backend.
     *
     * The range [0, 1000] is reserved for Louvre graphic backends only. If you wish to create
     * your custom graphic backends, use an ID out of that range.
     *
     * @enum LGraphicBackendID
     */
    enum LGraphicBackendID : UInt32
    {
        LGraphicBackendDRM = 0,     ///< ID for the DRM graphic backend.
        LGraphicBackendX11 = 1      ///< ID for the X11 graphic backend.
    };

    /**
     * @brief ID values for the input backends included with Louvre.
     *
     * These IDs are returned by LSeat::inputBackendId() to identify the currently
     * loaded input backend.
     *
     * The range [0, 1000] is reserved exclusively for Louvre's built-in input backends. If
     * you intend to create custom input backends, please use an ID outside of this range.
     *
     * @enum LInputBackendID
     */
    enum LInputBackendID : UInt32
    {
        LInputBackendLibinput = 0, ///< ID for the Libinput input backend.
        LInputBackendX11 = 1       ///< ID for the X11 input backend.
    };

    /**
     * @brief Structure representing a version in the format major.minor.patch-build.
     */
    struct LVersion
    {
        UInt32 major; ///< Major version.
        UInt32 minor; ///< Minor version.
        UInt32 patch; ///< Patch version.
        UInt32 build; ///< Build number.
    };

    namespace Protocols
    {
        namespace Wayland
        {
            class GCompositor;
            class GSubcompositor;
            class GSeat;
            class GDataDeviceManager;
            class GOutput;

            class RSurface;
            class RRegion;
            class RPointer;
            class RKeyboard;
            class RTouch;
            class RDataDevice;
            class RDataSource;
            class RDataOffer;
            class RSubsurface;
            class RCallback;
        }

        namespace XdgShell
        {
            class GXdgWmBase;

            class RXdgPositioner;
            class RXdgSurface;
            class RXdgToplevel;
            class RXdgPopup;
        };

        namespace XdgDecoration
        {
            class GXdgDecorationManager;

            class RXdgToplevelDecoration;
        };

        namespace WpPresentationTime
        {
            class GWpPresentation;

            class RWpPresentationFeedback;
        };

        namespace LinuxDMABuf
        {
            class GLinuxDMABuf;

            class RLinuxBufferParams;
            class RLinuxDMABufFeedback;
        };

        namespace Viewporter
        {
            class GViewporter;

            class RViewport;
        };

        namespace FractionalScale
        {
            class GFractionalScaleManager;

            class RFractionalScale;
        };

        namespace GammaControl
        {
            class GGammaControlManager;

            class RGammaControl;
        };

        namespace TearingControl
        {
            class GTearingControlManager;

            class RTearingControl;
        };

        namespace PointerGestures
        {
            class GPointerGestures;

            class RGestureSwipe;
            class RGesturePinch;
            class RGestureHold;
        };

        namespace RelativePointer
        {
            class GRelativePointerManager;

            class RRelativePointer;
        };
    }

    /// @cond OMIT
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
        EGLDisplay                          (*backendGetAllocatorEGLDisplay)();
        EGLContext                          (*backendGetAllocatorEGLContext)();

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

        /* MODE PROPS */
        const LSize *                       (*outputModeGetSize)(LOutputMode *mode);
        Int32                               (*outputModeGetRefreshRate)(LOutputMode *mode);
        bool                                (*outputModeIsPreferred)(LOutputMode *mode);
    };

    struct LInputBackendInterface
    {
        UInt32                             (*backendGetId)();
        void *                             (*backendGetContextHandle)();
        UInt32                             (*backendGetCapabilities)();
        const std::vector<LInputDevice*> * (*backendGetDevices)();
        bool                               (*backendInitialize)();
        void                               (*backendUninitialize)();
        void                               (*backendSuspend)();
        void                               (*backendResume)();
        void                               (*backendSetLeds)(UInt32);
        void                               (*backendForceUpdate)();
    };

    inline const std::string getenvString(const char *env)
    {
        const char *val { getenv(env) };

        if (val != NULL)
            return std::string(val);

        return std::string();
    }
    /// @endcond
};

#endif // LNAMESPACES_H
