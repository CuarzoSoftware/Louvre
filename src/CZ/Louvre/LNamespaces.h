#ifndef LNAMESPACES_H
#define LNAMESPACES_H

#include <CZ/CZ.h>
#include <CZ/Louvre/Protocols/Wayland/wayland.h>

#define LOUVRE_ASSERT_CHECKS 0
#define LOUVRE_MAX_SURFACE_SIZE 21474836
#define LOUVRE_MAX_DAMAGE_RECTS 128

/* Protocol Global Versions */
#define LOUVRE_FRACTIONAL_SCALE_MANAGER_VERSION 1
#define LOUVRE_WL_COMPOSITOR_VERSION 6
#define LOUVRE_WL_DATA_DEVICE_MANAGER_VERSION 3
#define LOUVRE_WL_SEAT_VERSION 9
#define LOUVRE_WL_OUTPUT_VERSION 4
#define LOUVRE_WL_SUBCOMPOSITOR_VERSION 1
#define LOUVRE_XDG_ACTIVATION_VERSION 1
#define LOUVRE_XDG_WM_BASE_VERSION 6
#define LOUVRE_XDG_DECORATION_MANAGER_VERSION 1
#define LOUVRE_XDG_OUTPUT_MANAGER_VERSION 3
#define LOUVRE_PRESENTATION_VERSION 1
#define LOUVRE_LINUX_DMA_BUF_VERSION 5
#define LOUVRE_VIEWPORTER_VERSION 1
#define LOUVRE_GAMMA_CONTROL_MANAGER_VERSION 1
#define LOUVRE_TEARING_CONTROL_MANAGER_VERSION 1
#define LOUVRE_RELATIVE_POINTER_MANAGER_VERSION 1
#define LOUVRE_POINTER_GESTURES_VERSION 3
#define LOUVRE_SESSION_LOCK_MANAGER_VERSION 1
#define LOUVRE_POINTER_CONSTRAINTS_VERSION 1
#define LOUVRE_SCREEN_COPY_MANAGER_VERSION 3
#define LOUVRE_OUTPUT_IMAGE_CAPTURE_SOURCE_MANAGER 1
#define LOUVRE_FOREIGN_TOPLEVEL_IMAGE_CAPTURE_SOURCE_MANAGER 1
#define LOUVRE_LAYER_SHELL_VERSION 5
#define LOUVRE_FOREIGN_TOPLEVEL_MANAGER_VERSION 3
#define LOUVRE_FOREIGN_TOPLEVEL_LIST_VERSION 1
#define LOUVRE_SINGLE_PIXEL_BUFFER_MANAGER_VERSION 1
#define LOUVRE_CONTENT_TYPE_MANAGER_VERSION 1
#define LOUVRE_IDLE_NOTIFIER_VERSION 1
#define LOUVRE_IDLE_INHIBIT_MANAGER_VERSION 1
#define LOUVRE_DRM_LEASE_DEVICE_VERSION 1
#define LOUVRE_WLR_OUTPUT_MANAGER_VERSION 4
#define LOUVRE_SVG_PATH_MANAGER_VERSION 1
#define LOUVRE_BACKGROUND_BLUR_MANAGER_VERSION 1
#define LOUVRE_INVISIBLE_REGION_MANAGER_VERSION 1

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

#define LCLASS_NO_COPY(class_name) \
    class_name(const class_name&) = delete; \
    class_name(class_name&&) = delete; \
    class_name &operator=(const class_name&) = delete;

using namespace CZ;

/**
 * @brief Namespaces
 */
namespace Louvre
{
    class LObject;
    class LFactoryObject;
    class LGlobal;
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
    class LSessionLockManager;
    class LSurface;
    class LTexture;
    class LScreenshotRequest;
    class LActivationTokenManager;
    class LActivationToken;
    class LBackgroundBlur;

    class LPainter;
    class LRenderBuffer;
    class LFramebuffer;
    class LOutputFramebuffer;
    class LFramebufferWrapper;

    // Data
    class LDND;
    class LDNDSession;
    class LClipboard;

    class LBaseSurfaceRole;
    class LCursorRole;
    class LDNDIconRole;
    class LToplevelRole;
    class LPopupRole;
    class LSubsurfaceRole;
    class LSessionLockRole;
    class LLayerRole;

    class LCursor;
    class LClientCursor;
    class LXCursor;

    class LSeat;
    class LPointer;
    class LKeyboard;
    class LTouch;

    class LGPU;
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
    class LForeignToplevelController;

    // Other
    class LDMABuffer;
    class LSinglePixelBuffer;
    class LExclusiveZone;
    class LIdleListener;

    // Utils
    class LLog;
    class LTime;
    class LTimer;
    class LLauncher;
    class LGammaTable;
    template <class T> class LPointTemplate;
    template <class T> class LRectTemplate;

    /// @brief 24 bits Wayland float
    typedef wl_fixed_t      Float24;

    /// @brief Unsigned integer capable of holding a pointer
    typedef uintptr_t       UIntPtr;

    /// 2D vector of 32 bits integers
    using LPoint = LPointTemplate<Int32>;

    /// 2D vector of 32 bits integers
    using LSize = LPoint;

    /// 2D vector of 32 bits floats
    using LPointF = LPointTemplate<Float32>;

    /// 2D vector of 32 bits floats
    using LSizeF = LPointF;

    /// 4D vector of 32 bits integers
    using LRect = LRectTemplate<Int32>;

    /// 4D vector of 32 bits floats
    using LRectF = LRectTemplate<Float32>;

    class LRRect;

    typedef void* EGLContext;
    typedef void* EGLDisplay;
    typedef unsigned int GLuint;
    typedef unsigned int GLenum;

    struct LRGBF;
    struct LRGBAF;
    struct LBlendFunc;
    struct LDMAFormat;
    struct LDMAPlanes;
    struct LBox;
    struct LMargins;

    /**
     * @brief Graphic backend IDs.
     *
     * Use LCompositor::graphicBackendId() to identify the currently loaded backend.
     */
    enum LGraphicBackendID : UInt32
    {
        LGraphicBackendDRM = 0,    ///< ID for the DRM graphic backend.
        LGraphicBackendWayland = 1 ///< ID for the Wayland graphic backend.
    };

    /**
     * @brief Input backend IDs.
     *
     * Use LCompositor::inputBackendId() to identify the currently loaded backend.
     */
    enum LInputBackendID : UInt32
    {
        LInputBackendLibinput = 0, ///< ID for the Libinput input backend.
        LInputBackendWayland = 1   ///< ID for the Wayland input backend.
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

    /**
     * @brief Image capture source type
     */
    enum class LImageCaptureSourceType : UInt32
    {
        Output,         ///< LOutput
        ForeignToplevel ///< LToplevelRole
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

        namespace XdgActivation
        {
            class GXdgActivation;

            class RXdgActivationToken;
        };

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

        namespace XdgOutput
        {
            class GXdgOutputManager;

            class RXdgOutput;
        }

        namespace PresentationTime
        {
            class GPresentation;

            class RPresentationFeedback;
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

        namespace SessionLock
        {
            class GSessionLockManager;

            class RSessionLock;
            class RSessionLockSurface;
        };

        namespace ScreenCopy
        {
            class GScreenCopyManager;

            class RScreenCopyFrame;
        };

        namespace ImageCaptureSource
        {
            class GOutputImageCaptureSourceManager;
            class GForeignToplevelImageCaptureSourceManager;

            class RImageCaptureSource;
        }

        namespace PointerConstraints
        {
            class GPointerConstraints;

            class RLockedPointer;
            class RConfinedPointer;
        };

        namespace LayerShell
        {
            class GLayerShell;

            class RLayerSurface;
        }

        namespace ForeignToplevelManagement
        {
            class GForeignToplevelManager;

            class RForeignToplevelHandle;
        }

        namespace ForeignToplevelList
        {
            class GForeignToplevelList;

            class RForeignToplevelHandle;
        }

        namespace SinglePixelBuffer
        {
            class GSinglePixelBufferManager;
        }

        namespace ContentType
        {
            class GContentTypeManager;

            class RContentType;
        }

        namespace IdleNotify
        {
            class GIdleNotifier;

            class RIdleNotification;
        }

        namespace IdleInhibit
        {
            class GIdleInhibitManager;

            class RIdleInhibitor;
        }

        namespace DRMLease
        {
            class GDRMLeaseDevice;

            class RDRMLeaseConnector;
            class RDRMLeaseRequest;
            class RDRMLease;
        }

        namespace WlrOutputManagement
        {
            class GWlrOutputManager;

            class RWlrOutputHead;
            class RWlrOutputMode;
            class RWlrOutputConfiguration;
            class RWlrOutputConfigurationHead;
        }

        namespace BackgroundBlur
        {
            class GBackgroundBlurManager;

            class RBackgroundBlur;
        }

        namespace SvgPath
        {
            class GSvgPathManager;

            class RSvgPath;
        }

        namespace InvisibleRegion
        {
            class GInvisibleRegionManager;

            class RInvisibleRegion;
        }
    }

    /**
     * @brief Gets the static LCompositor instance.
     *
     * This method provides access to the single LCompositor instance that can exist per process.\n
     *
     * @return A pointer to the LCompositor instance or `nullptr` if not yet created.
     */
    LCompositor *compositor() noexcept;

    /**
     * @brief Gets the compositor's cursor.
     *
     * @warning Must be accessed within or after the LCompositor::initialized() or LCompositor::cursorInitialized() events.
     *
     * @return A pointer to the LCursor instance or `nullptr` if not yet initialized.
     */
    LCursor *cursor() noexcept;

    /**
     * @brief Gets the compositor's seat.
     *
     * The seat provides access to the LClipboard, LDND, LPointer, LKeyboard, LTouch, and LOutput instances.
     *
     * @warning Must be accessed within or after the LCompositor::initialized() event.
     *
     * @return A pointer to the LSeat instance.
     */
    LSeat *seat() noexcept;

    /**
     * @brief Gets the compositor's session lock manager.
     *
     * @warning Must be accessed within or after the LCompositor::initialized() event.
     */
    LSessionLockManager *sessionLockManager() noexcept;

    /**
     * @brief Provides access to the activation token manager.
     *
     * @warning Must be accessed within or after the LCompositor::initialized() event.
     */
    LActivationTokenManager *activationTokenManager() noexcept;
};

#endif // LNAMESPACES_H
