#ifndef DMA_H
#define DMA_H

#include <EGL/egl.h>
#include <LNamespaces.h>

using namespace Louvre;

struct LDMAPlane
{
    Int32 fd;
    UInt32 index;
    UInt32 offset;
    UInt32 stride;
    UInt64 modifier;
};

struct LDMAParams
{
    std::list<LDMAPlane> planes;
    wl_resource *resource;
    LClient *client;
};

struct LDMABuffer
{
    Int32 width;
    Int32 height;
    UInt32 format;
    UInt32 flags;
    EGLImage eglImage;
    wl_resource *resource;
};

bool wl_buffer_is_dmabuf(wl_resource *resource);

#endif // DMA_H
