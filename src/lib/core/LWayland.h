#ifndef LWAYLAND_H
#define LWAYLAND_H

#include <LNamespaces.h>

class Louvre::LWayland
{
public:
    static wl_event_source *addFdListener(int fd, void *userData, int(*callback)(int,unsigned int,void*), UInt32 flags = WL_EVENT_READABLE);
    static void removeFdListener(wl_event_source *source);

    static void setSeat(LSeat *seat);
    static int processSeat(int, unsigned int, void*userData);

    static UInt32 nextSerial();
    static bool wlFormat2Gl(UInt32 wlFormat, GLenum *glFormat, GLenum *glType);
    static int initWayland(LCompositor *comp);
    static void terminateDisplay();
    static void dispatchEvents();
    static void flushClients();
    static void scheduleDraw(LCompositor *comp);
    static void bindEGLDisplay(EGLDisplay eglDisplay);
    static void runLoop();
    static int drmFd();

    static void clientConnectionEvent(wl_listener *listener, void *data);
    static void clientDisconnectionEvent(wl_listener *listener, void *data);

    static int apply_damage_emit(void *data);
    static void clDisc(wl_listener *listener, void *data);

    static wl_event_source *addTimer(wl_event_loop_timer_func_t func, void *data);

    static wl_display *getDisplay();
    static EGLContext eglContext();
    static EGLDisplay eglDisplay();
    static void initGLContext();
    static bool isGlContextInitialized();

    static void setContext(const LOutput *output, EGLDisplay sharedDisplay, EGLContext sharedContext);
    static void setMainOutput(LOutput *output);

    static const LOutput *mainOutput();

    static LCompositor *bindedCompositor();
    static void forceUpdate();




};



#endif // LWAYLAND_H
