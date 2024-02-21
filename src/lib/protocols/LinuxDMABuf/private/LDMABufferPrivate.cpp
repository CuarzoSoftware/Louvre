#include <protocols/LinuxDMABuf/private/LDMABufferPrivate.h>

void LDMABuffer::LDMABufferPrivate::destroy(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);
    wl_resource_destroy(resource);   
}
