#include <X11/Xlib.h>
#include <X11/extensions/Xfixes.h>
#include <X11/extensions/Xrandr.h>
#include <linux/input.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <sys/mman.h>
#include <sys/timerfd.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <poll.h>

#include <LGraphicBackend.h>
#include <private/LCompositorPrivate.h>
#include <private/LOutputPrivate.h>
#include <private/LOutputModePrivate.h>
#include <LWayland.h>
#include <LCursor.h>
#include <LLog.h>

using namespace Louvre;

std::list<LOutput*>outputs;

struct X11_Window
{
    Window window;
    EGLContext context;
    EGLSurface surface;
};

struct OUTPUT_MODE
{
    LSize size;
    Int32 refreshRate;
    bool isPreferred = true;
};

struct OUTPUT_DATA
{
    Int32 backendId = 1;
    Display *x_display;
    X11_Window window;
    EGLConfig config;
    EGLint num_configs_returned;
    EGLDisplay egl_display;
    LSize physicalSize;
    list<LOutputMode*>modes;
    bool initialized = false;
    Int32 currentBufferIndex = 0;
};


bool LGraphicBackend::initialize(LCompositor *compositor)
{
    LOutput *output = ((LCompositor*)compositor)->createOutputRequest();
    output->imp()->compositor = (LCompositor*)compositor;
    LOutputMode *mode = new LOutputMode(output);
    OUTPUT_MODE *modeData = new OUTPUT_MODE;
    mode->imp()->graphicBackendData = modeData;
    OUTPUT_DATA *data = new OUTPUT_DATA;
    data->modes.push_back(mode);
    output->imp()->graphicBackendData = data;
    data->x_display = XOpenDisplay(NULL);
    data->egl_display = eglGetDisplay(data->x_display);
    outputs.push_back(output);

    static const EGLint attribs[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RED_SIZE, 1,
        EGL_GREEN_SIZE, 1,
        EGL_BLUE_SIZE, 1,
        EGL_ALPHA_SIZE, 0,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_NONE
    };

    //const char *extensions = eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);

    //printf("%s", extensions);

    eglInitialize(data->egl_display, NULL, NULL);
    eglChooseConfig(data->egl_display, attribs, &data->config, 1, &data->num_configs_returned);

    // get the visual from the EGL config
    EGLint visual_id;
    eglGetConfigAttrib(data->egl_display, data->config, EGL_NATIVE_VISUAL_ID, &visual_id);
    XVisualInfo visual_template;
    visual_template.visualid = visual_id;
    int num_visuals_returned;
    XVisualInfo *visual = XGetVisualInfo(data->x_display, VisualIDMask, &visual_template, &num_visuals_returned);

    // Get the screen size
    int snum = DefaultScreen(data->x_display);

#if LOUVRE_DEBUG == 0
    modeData->size.setW(DisplayWidth(data->x_display, snum));
    modeData->size.setH(DisplayHeight(data->x_display, snum));
#else
    modeData->size.setW(DisplayWidth(data->x_display, snum)/2);
    modeData->size.setH(DisplayHeight(data->x_display, snum)/2);
#endif


    // create a window
    XSetWindowAttributes window_attributes;
    window_attributes.override_redirect = True;
    window_attributes.event_mask = ExposureMask | StructureNotifyMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | EnterWindowMask | LeaveWindowMask | FocusChangeMask;
    window_attributes.colormap = XCreateColormap(data->x_display, RootWindow(data->x_display, DefaultScreen(data->x_display)), visual->visual, AllocNone);
    data->window.window = XCreateWindow (
        data->x_display,
        RootWindow(data->x_display, DefaultScreen(data->x_display)),
        0, 0,
        modeData->size.w(), modeData->size.h(),
        0, // border width
        visual->depth, // depth
        InputOutput, // class
        visual->visual, // visual
        CWColormap | CWEventMask | CWOverrideRedirect, // attribute mask
        &window_attributes // attributes
    );

    // Obtiene el tamaño físico de la salida en milímetros
    XRRScreenResources *res = XRRGetScreenResourcesCurrent(data->x_display, data->window.window);
    XRROutputInfo *output_info = XRRGetOutputInfo(data->x_display, res, res->outputs[0]);
    data->physicalSize.setW(output_info->mm_width);
    data->physicalSize.setH(output_info->mm_height);

    XRRScreenConfiguration *conf_info = XRRGetScreenInfo(data->x_display, data->window.window);
    modeData->refreshRate = XRRConfigCurrentRate(conf_info)*1000;

    XRRFreeScreenConfigInfo(conf_info);
    XRRFreeOutputInfo(output_info);
    XRRFreeScreenResources(res);


    XFree(visual);

    // EGL context and surface
    if(!eglBindAPI(EGL_OPENGL_ES_API))
    {
        printf("Failed to bind api EGL_OPENGL_ES_API\n");
        exit(-1);
    }

    return true;
}

void LGraphicBackend::uninitialize(const LCompositor *compositor)
{
    L_UNUSED(compositor);

    /* TODO: Permitir cambiar de backend gráfico dinámicamente ? */
}

const list<LOutput*> *LGraphicBackend::getAvaliableOutputs(const LCompositor *compositor)
{
    L_UNUSED(compositor);

    return &outputs;
}

void LGraphicBackend::initializeOutput(const LOutput *output)
{

    OUTPUT_DATA *data = (OUTPUT_DATA*)output->imp()->graphicBackendData;

    EGLContext ctx = EGL_NO_CONTEXT;

    if(LWayland::isGlContextInitialized())
        ctx = LWayland::eglContext();

    static const EGLint context_attribs[] =
    {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };

    data->window.context = eglCreateContext(data->egl_display, data->config, ctx, context_attribs);

    if(data->window.context == NULL)
    {
        printf("Failed to create context.\n");
        exit(-1);
    }

    data->window.surface = eglCreateWindowSurface(data->egl_display, data->config, data->window.window, NULL);

    if(data->window.surface == EGL_NO_SURFACE)
    {
        printf("Failed to create egl surface.\n");
        exit(-1);
    }

    eglMakeCurrent(data->egl_display, data->window.surface, data->window.surface, data->window.context);

    XMapWindow(data->x_display, data->window.window);
    XMoveWindow(data->x_display, data->window.window, 0, 0);

#if LOUVRE_DEBUG == 0
    XFixesHideCursor(data->x_display, data->window.window);
    XSetInputFocus(data->x_display, data->window.window, RevertToParent, CurrentTime);
#endif

    if(!LWayland::isGlContextInitialized())
        LWayland::setContext(output, data->egl_display, data->window.context);


    data->initialized = true;
    output->imp()->state = LOutput::Initialized;

}

void LGraphicBackend::uninitializeOutput(const LOutput *output)
{
    OUTPUT_DATA *data = (OUTPUT_DATA*)output->imp()->graphicBackendData;

    // Si no estaba inicializada no hacemos nada
    if(data->initialized)
    {
        // Destruimos la superficie EGL
        eglDestroySurface(data->egl_display, data->window.surface);

        // Destruimos el contexto EGL
        eglDestroyContext(data->egl_display, data->window.context);
    }

}

void LGraphicBackend::flipOutputPage(const LOutput *output)
{
    output->imp()->compositor->imp()->renderMutex.unlock();
    OUTPUT_DATA *data = (OUTPUT_DATA*)output->imp()->graphicBackendData;
    eglSwapBuffers(data->egl_display, data->window.surface);

    EGLint buffer_age = 0;

    if(eglQuerySurface(data->egl_display, data->window.surface, EGL_BUFFER_AGE_EXT, &buffer_age) == EGL_FALSE)
    {
        data->currentBufferIndex = 1 - data->currentBufferIndex;
    }
    else
    {
        data->currentBufferIndex = buffer_age;
    }

    LWayland::forceUpdate();
}

EGLDisplay LGraphicBackend::getOutputEGLDisplay(const LOutput *output)
{
    OUTPUT_DATA *data = (OUTPUT_DATA*)output->imp()->graphicBackendData;
    return data->egl_display;
}

const LSize *LGraphicBackend::getOutputPhysicalSize(const LOutput *output)
{
    OUTPUT_DATA *data = (OUTPUT_DATA*)output->imp()->graphicBackendData;
    return &data->physicalSize;
}

Int32 LGraphicBackend::getOutputCurrentBufferIndex(const LOutput *output)
{
    OUTPUT_DATA *data = (OUTPUT_DATA*)output->imp()->graphicBackendData;
    return data->currentBufferIndex;
}

const char *LGraphicBackend::getOutputName(const LOutput *output)
{
    L_UNUSED(output);
    return "X11 Window";
}

const char *LGraphicBackend::getOutputManufacturerName(const LOutput *output)
{
    L_UNUSED(output);
    return "Unknown Manufacturer";
}

const char *LGraphicBackend::getOutputModelName(const LOutput *output)
{
    L_UNUSED(output);
    return "X11 Fullscreen Window";
}

const char *LGraphicBackend::getOutputDescription(const LOutput *output)
{
    L_UNUSED(output);
    return "A X11 window.";
}

const LOutputMode *LGraphicBackend::getOutputPreferredMode(const LOutput *output)
{
    OUTPUT_DATA *data = (OUTPUT_DATA*)output->imp()->graphicBackendData;
    return data->modes.front();
}

const LOutputMode *LGraphicBackend::getOutputCurrentMode(const LOutput *output)
{
    OUTPUT_DATA *data = (OUTPUT_DATA*)output->imp()->graphicBackendData;
    return data->modes.front();
}

const std::list<LOutputMode *> *LGraphicBackend::getOutputModes(const LOutput *output)
{
    OUTPUT_DATA *data = (OUTPUT_DATA*)output->imp()->graphicBackendData;
    return &data->modes;
}

void LGraphicBackend::setOutputMode(const LOutput *output, const LOutputMode *mode)
{
    OUTPUT_DATA *data = (OUTPUT_DATA*)output->imp()->graphicBackendData;

    L_UNUSED(mode);
    L_UNUSED(data);

    /* TODO: Permitir definir modos custom. */
}

const LSize *LGraphicBackend::getOutputModeSize(const LOutputMode *mode)
{
    OUTPUT_MODE *data = (OUTPUT_MODE*)mode->imp()->graphicBackendData;
    return &data->size;
}

Int32 LGraphicBackend::getOutputModeRefreshRate(const LOutputMode *mode)
{
    OUTPUT_MODE *data = (OUTPUT_MODE*)mode->imp()->graphicBackendData;
    return data->refreshRate;
}

bool LGraphicBackend::getOutputModeIsPreferred(const LOutputMode *mode)
{
    OUTPUT_MODE *data = (OUTPUT_MODE*)mode->imp()->graphicBackendData;
    return data->isPreferred;
}

void LGraphicBackend::initializeCursor(const LOutput *output)
{
    L_UNUSED(output);
}

bool LGraphicBackend::hasHardwareCursorSupport()
{
    return false;
}

void LGraphicBackend::setCursorTexture(const LOutput *output, const LTexture *texture, const LSizeF &size)
{
    L_UNUSED(output);
    L_UNUSED(texture);
    L_UNUSED(size);
}

void LGraphicBackend::setCursorPosition(const LOutput *output, const LPoint &position)
{
    L_UNUSED(output);
    L_UNUSED(position);
}

LGraphicBackendInterface API;

extern "C" LGraphicBackendInterface *getAPI()
{
    API.initialize = &LGraphicBackend::initialize;
    API.uninitialize = &LGraphicBackend::uninitialize;
    API.getAvaliableOutputs = &LGraphicBackend::getAvaliableOutputs;
    API.initializeOutput = &LGraphicBackend::initializeOutput;
    API.uninitializeOutput = &LGraphicBackend::uninitializeOutput;
    API.flipOutputPage = &LGraphicBackend::flipOutputPage;
    API.getOutputEGLDisplay = &LGraphicBackend::getOutputEGLDisplay;
    API.getOutputPhysicalSize = &LGraphicBackend::getOutputPhysicalSize;
    API.getOutputCurrentBufferIndex = &LGraphicBackend::getOutputCurrentBufferIndex;
    API.getOutputName = &LGraphicBackend::getOutputName;
    API.getOutputManufacturerName = &LGraphicBackend::getOutputManufacturerName;
    API.getOutputModelName = &LGraphicBackend::getOutputModelName;
    API.getOutputDescription = &LGraphicBackend::getOutputDescription;
    API.getOutputPreferredMode = &LGraphicBackend::getOutputPreferredMode;
    API.getOutputCurrentMode = &LGraphicBackend::getOutputCurrentMode;
    API.getOutputModes = &LGraphicBackend::getOutputModes;
    API.setOutputMode = &LGraphicBackend::setOutputMode;
    API.getOutputModeSize = &LGraphicBackend::getOutputModeSize;
    API.getOutputModeRefreshRate = &LGraphicBackend::getOutputModeRefreshRate;
    API.getOutputModeIsPreferred = &LGraphicBackend::getOutputModeIsPreferred;
    API.initializeCursor = &LGraphicBackend::initializeCursor;
    API.hasHardwareCursorSupport = &LGraphicBackend::hasHardwareCursorSupport;
    API.setCursorTexture = &LGraphicBackend::setCursorTexture;
    API.setCursorPosition = &LGraphicBackend::setCursorPosition;
    return &API;
}
