#include <protocols/Wayland/private/RCallbackPrivate.h>

using namespace Louvre::Protocols::Wayland;

RCallback::RCallback
(
    wl_client *client,
    UInt32 id,
    std::list<RCallback *> *list
)
    :LResource
    (
        client,
        &wl_callback_interface,
        LOUVRE_WL_CALLBACK_VERSION,
        id,
        NULL,
        &RCallbackPrivate::resource_destroy
    )
{
    m_imp = new RCallbackPrivate();

    if (list)
    {
        imp()->list = list;
        imp()->list->push_back(this);
        imp()->listLink = std::prev(imp()->list->end());
    }
}

RCallback::~RCallback()
{
    if (imp()->list)
        imp()->list->erase(imp()->listLink);

    delete m_imp;
}

bool RCallback::done(UInt32 data)
{
    wl_callback_send_done(resource(), data);
    return true;
}
