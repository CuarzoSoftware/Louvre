#include <protocols/LinuxDMABuf/private/LDMABufferPrivate.h>

void LDMABuffer::LDMABufferPrivate::resource_destroy(wl_resource *resource)
{
    LDMABuffer *lDMABuffer = (LDMABuffer*)wl_resource_get_user_data(resource);
    delete lDMABuffer;
}

void LDMABuffer::LDMABufferPrivate::destroy(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);
    wl_resource_destroy(resource);
}
