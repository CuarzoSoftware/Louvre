#ifndef LINPUTBACKEND
#define LINPUTBACKEND

#include <LNamespaces.h>

class Louvre::LInputBackend
{
public:
    static UInt32                               backendGetId();
    static UInt32                               backendGetCapabilities();
    static void *                               backendGetContextHandle();
    static const std::vector<LInputDevice*> *   backendGetDevices();
    static bool                                 backendInitialize();
    static void                                 backendUninitialize();
    static void                                 backendSuspend();
    static void                                 backendResume();
    static void                                 backendForceUpdate();
    static Int32                                processInput(int, unsigned int, void *);
};

#endif // LINPUTBACKEND
