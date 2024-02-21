#include <protocols/Wayland/private/RCallbackPrivate.h>

using namespace Louvre::Protocols::Wayland;

RCallback::RCallback
(
    wl_client *client,
    UInt32 id,
    std::vector<RCallback *> *vec
)
    :LResource
    (
        client,
        &wl_callback_interface,
        LOUVRE_WL_CALLBACK_VERSION,
        id,
        NULL
    ),
    LPRIVATE_INIT_UNIQUE(RCallback)
{
    if (vec)
    {
        imp()->vec = vec;
        imp()->vec->push_back(this);
    }
}

RCallback::~RCallback()
{
    if (imp()->vec)
        LVectorRemoveOne(*imp()->vec, this);
}

bool RCallback::done(UInt32 data)
{
    wl_callback_send_done(resource(), data);
    return true;
}
