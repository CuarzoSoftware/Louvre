#ifndef LDMABUFFERPRIVATE_H
#define LDMABUFFERPRIVATE_H

#include <protocols/LinuxDMABuf/LDMABuffer.h>

using namespace Louvre;
using namespace std;

LPRIVATE_CLASS(LDMABuffer)
    static void destroy(wl_client *client, wl_resource *resource);
    LTexture *texture { nullptr };
    LDMAPlanes *planes { nullptr };
};

#endif // LDMABUFFERPRIVATE_H
