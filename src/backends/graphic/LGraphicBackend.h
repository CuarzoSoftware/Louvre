#ifndef LGRAPHICBACKEND
#define LGRAPHICBACKEND

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <LNamespaces.h>
#include <list>

class Louvre::LGraphicBackend
{
public:

    // Inicializa el báckend
    static bool initialize(LCompositor *compositor);

    // Desinicializa el báckend
    static void uninitialize(const LCompositor *compositor);

    // Lista de salidas disponibles
    static const std::list<LOutput*>*getAvaliableOutputs(const LCompositor *compositor);

    /* OUTPUTS */

    // Inicializa una salida
    static void initializeOutput(const LOutput *output);

    // Desinicializa una salida
    static void uninitializeOutput(const LOutput *output);

    // Realiza un pageflipping de la salida
    static void flipOutputPage(const LOutput *output);

    // Handle al EGLDisplay de una salida
    static EGLDisplay getOutputEGLDisplay(const LOutput *output);

    // Dimensiones de una salida en milímetros
    static const LSize *getOutputPhysicalSize(const LOutput *output);

    // Índice del buffer actual
    static Int32 getOutputCurrentBufferIndex(const LOutput *output);

    // Nombre de la salida (HDMI-A-1, eDP-1, etc)
    static const char *getOutputName(const LOutput *output);

    // Fabricante de la salida
    static const char *getOutputManufacturerName(const LOutput *output);

    // Modelo de la salida
    static const char *getOutputModelName(const LOutput *output);

    // Descripción de la salida
    static const char *getOutputDescription(const LOutput *output);

    // Modo preferido de una salida
    static const LOutputMode *getOutputPreferredMode(const LOutput *output);

    // Modo actual de una salida
    static const LOutputMode *getOutputCurrentMode(const LOutput *output);

    // Modos de una salida
    static const std::list<LOutputMode*> *getOutputModes(const LOutput *output);

    // Asigna un modo a una salida
    static void setOutputMode(const LOutput *output, const LOutputMode *mode);


    /* OUTPUT MODES */

    // Dimensiones de un modo
    static const LSize *getOutputModeSize(const LOutputMode *mode);

    // Tasa de refresco de un modo
    static Int32 getOutputModeRefreshRate(const LOutputMode *mode);

    // Indica si es un modo preferido
    static bool getOutputModeIsPreferred(const LOutputMode *mode);


    /* CURSOR */

    // Inicializa un cursor
    static void initializeCursor(const LOutput *output);

    // Indica si el backend soporta composición del cursor vía hardware
    static bool hasHardwareCursorSupport();

    // Asigna una textura al cursor
    static void setCursorTexture(const LOutput *output, const LTexture *texture, const LSizeF &size);

    // Asigna la posición del cursor
    static void setCursorPosition(const LOutput *output, const LPoint &position);
};


#endif // LGRAPHICBACKEND
