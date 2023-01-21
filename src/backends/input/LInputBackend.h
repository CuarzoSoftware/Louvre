#ifndef LINPUTBACKEND
#define LINPUTBACKEND

#include <LNamespaces.h>

class Louvre::LInputBackend
{
public:

    // Inicializa el báckend
    static bool initialize(const LSeat *seat);

    // Capacidades
    static UInt32 getCapabilities(const LSeat *seat);

    // Capacidades
    static void *getContextHandle(const LSeat *seat);

    // Suspende los eventos
    static void suspend(const LSeat *seat);

    // Fuerza procesamiento de ventos
    static void forceUpdate(const LSeat *seat);

    // Resume los eventos
    static void resume(const LSeat *seat);

    // De-inicializa el báckend
    static void uninitialize(const LSeat *seat);

};


#endif // LINPUTBACKEND
