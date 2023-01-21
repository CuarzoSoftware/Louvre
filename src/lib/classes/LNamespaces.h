#ifndef LNAMESPACES_H
#define LNAMESPACES_H

#include <globals/Wayland/wayland.h>
#include <libinput.h>
#include <list>

#define MAX_SURFACE_SIZE 10000000
#define LOUVRE_DEBUG 0

// Globals
#define LOUVRE_COMPOSITOR_VERSION 5
#define LOUVRE_SEAT_VERSION 7
#define LOUVRE_OUTPUT_VERSION 3 // Last is 4
#define LOUVRE_SUBCOMPOSITOR_VERSION 1
#define LOUVRE_DATA_DEVICE_MANAGER_VERSION 3

#define LOUVRE_XDG_WM_BASE_VERSION 5
#define LOUVRE_XDG_DECORATION_MANAGER_VERSION 1

#define L_UNUSED(object){(void)object;}

/*!
 * @brief Namespaces
 */
namespace Louvre
{
    // API Classes
    class LGraphicBackend;
    class LInputBackend;
    class LClient;
    class LCompositor;
    class LOutput;
    class LOutputManager;
    class LOutputMode;
    class LOpenGL;
    class LPainter;
    class LPositioner;
    class LRegion;
    class LSurface;
    class LSession;
    class LTexture;
    class LWayland;

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
    class LCursor;
    class LXCursor;

    // Utils
    class LLog;
    class LTime;
    template <class TA, class TB> class LPointTemplate;
    template <class TA, class TB> class LRectTemplate;

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
    typedef unsigned char   UChar8;

    /// @brief 8 bits signed integer
    typedef char            Char8;

    /// @brief 64 bits float
    typedef double          Float64;

    /// @brief 32 bits float
    typedef float           Float32;

    /// 2D vector of 32 bits integers
    typedef LPointTemplate<Int32,Float32> LPoint;

    /// 2D vector of 32 bits integers
    typedef LPoint LSize;

    /// 2D vector of 32 bits floats
    typedef LPointTemplate<Float32,Int32> LPointF;

    /// 2D vector of 32 bits floats
    typedef LPointF LSizeF;

    /// 4D vector of 32 bits integers
    typedef LRectTemplate<Int32,Float32> LRect;

    /// 4D vector of 32 bits floats
    typedef LRectTemplate<Float32,Int32> LRectF;

    typedef UInt32 LKey;

    typedef void* EGLContext;
    typedef void* EGLDisplay;
    typedef unsigned int GLuint;
    typedef unsigned int GLenum;

    struct LGraphicBackendInterface
    {
        bool (*initialize)(LCompositor *compositor);
        void (*uninitialize)(const LCompositor *compositor);
        const std::list<LOutput*>*(*getAvaliableOutputs)(const LCompositor *compositor);
        void (*initializeOutput)(const LOutput *output);
        void (*uninitializeOutput)(const LOutput *output);
        void (*flipOutputPage)(const LOutput *output);
        EGLDisplay (*getOutputEGLDisplay)(const LOutput *output);
        const LSize *(*getOutputPhysicalSize)(const LOutput *output);
        Int32 (*getOutputCurrentBufferIndex)(const LOutput *output);
        const char *(*getOutputName)(const LOutput *output);
        const char *(*getOutputManufacturerName)(const LOutput *output);
        const char *(*getOutputModelName)(const LOutput *output);
        const char *(*getOutputDescription)(const LOutput *output);
        const LOutputMode *(*getOutputPreferredMode)(const LOutput *output);
        const LOutputMode *(*getOutputCurrentMode)(const LOutput *output);
        const std::list<LOutputMode*> *(*getOutputModes)(const LOutput *output);
        void (*setOutputMode)(const LOutput *output, const LOutputMode *mode);
        const LSize *(*getOutputModeSize)(const LOutputMode *mode);
        Int32 (*getOutputModeRefreshRate)(const LOutputMode *mode);
        bool (*getOutputModeIsPreferred)(const LOutputMode *mode);
        void (*initializeCursor)(const LOutput *output);
        bool (*hasHardwareCursorSupport)();
        void (*setCursorTexture)(const LOutput *output, const LTexture *texture, const LSizeF &size);
        void (*setCursorPosition)(const LOutput *output, const LPoint &position);
    };

    struct LInputBackendInterface
    {
        bool (*initialize)(const LSeat *seat);
        UInt32 (*getCapabilities)(const LSeat *seat);
        void *(*getContextHandle)(const LSeat *seat);
        void (*uninitialize)(const LSeat *seat);
        void (*suspend)(const LSeat *seat);
        void (*forceUpdate)(const LSeat *seat);
        void (*resume)(const LSeat *seat);
    };


    // Wayland Globals
    namespace Globals
    {
        class Compositor;
        class Subcompositor;
        class DataDeviceManager;
        class DataDevice;
        class DataSource;
        class DataOffer;
        class Keyboard;
        class Output;
        class Pointer;
        class Region;
        class Seat;
        class Surface;
        class Subsurface;

        /// @brief Commit origin
        /// Indicates who requests to commit a surface
        enum CommitOrigin
        {
            /// @brief The commit is requested by the surface itself
            Itself,

            /// @brief The commit is requested by the parent surface
            Parent
        };
    };

    // Extensions Globals
    namespace Extensions
    {
        namespace XdgShell
        {
            class Popup;
            class Surface;
            class Toplevel;
            class WmBase;
            class Positioner;
        };

        namespace XdgDecoration
        {
            class Manager;
            class ToplevelDecoration;
        };

        namespace Viewporter
        {
            class Viewporter;
            class Viewport;
        };

        namespace LinuxDMABuf
        {
            class LinuxDMABuf;
            class Params;
            class Feedback;
        };
    };

};

#endif // LNAMESPACES_H
