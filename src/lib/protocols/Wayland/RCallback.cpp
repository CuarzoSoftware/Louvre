#include <protocols/Wayland/RCallback.h>

using namespace Louvre::Protocols::Wayland;

RCallback::RCallback
(
    wl_client *client,
    UInt32 id,
    std::vector<RCallback *> *vector
) noexcept
    :LResource
    (
        client,
        &wl_callback_interface,
        LOUVRE_WL_CALLBACK_VERSION,
        id,
        NULL
    ),
    m_vector(vector)
{
    if (m_vector)
        m_vector->push_back(this);
}

RCallback::~RCallback() noexcept
{
    if (m_vector)
        LVectorRemoveOne(*m_vector, this);
}

/******************** EVENTS ********************/

bool RCallback::done(UInt32 data) noexcept
{
    wl_callback_send_done(resource(), data);
    return true;
}
