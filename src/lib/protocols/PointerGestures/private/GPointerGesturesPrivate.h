#ifndef GPOINTERGESTURESPRIVATE_H
#define GPOINTERGESTURESPRIVATE_H

#include <protocols/PointerGestures/GPointerGestures.h>
#include <protocols/PointerGestures/pointer-gestures-unstable-v1.h>

using namespace Louvre::Protocols::PointerGestures;

LPRIVATE_CLASS(GPointerGestures)
static void bind(wl_client *client, void *data, UInt32 version, UInt32 id);
static void resource_destroy(wl_resource *resource);
static void get_swipe_gesture(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *pointer);
static void get_pinch_gesture(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *pointer);

#if LOUVRE_POINTER_GESTURES_VERSION >= 2
static void release(wl_client *client, wl_resource *resource);
#endif

#if LOUVRE_POINTER_GESTURES_VERSION >= 3
static void get_hold_gesture(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *pointer);
#endif

std::list<GPointerGestures*>::iterator clientLink;
};

#endif // GPOINTERGESTURESPRIVATE_H
