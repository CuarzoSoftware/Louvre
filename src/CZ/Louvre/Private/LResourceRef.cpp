#include <CZ/Louvre/Private/LResourceRef.h>

using namespace CZ;

LResourceRef::LResourceRef(wl_resource *res) noexcept
{
    m_onDestroy.notify = &HandleDestroy;
    setResource(res);
}

LResourceRef::~LResourceRef() noexcept
{
    setResource(nullptr);
}

LResourceRef::LResourceRef(const LResourceRef &other) noexcept
{
    m_onDestroy.notify = &HandleDestroy;
    setResource(other.m_res);
}

LResourceRef::LResourceRef(LResourceRef &&other) noexcept
{
    m_onDestroy.notify = &HandleDestroy;
    setResource(other.m_res);
    other.setResource(nullptr);
}

LResourceRef &LResourceRef::operator=(const LResourceRef &other) noexcept
{
    if (this != &other)
        setResource(other.m_res);

    return *this;
}

LResourceRef &LResourceRef::operator=(LResourceRef &&other) noexcept
{
    if (this != &other)
    {
        setResource(other.m_res);
        other.setResource(nullptr);
    }

    return *this;
}

void LResourceRef::setResource(wl_resource *resource) noexcept
{
    if (m_res == resource) return;

    if (m_res)
        wl_list_remove(&m_onDestroy.link);

    m_res = resource;

    if (m_res)
        wl_resource_add_destroy_listener(m_res, &m_onDestroy);
}

void LResourceRef::HandleDestroy(wl_listener *listener, void */*data*/) noexcept
{
    auto *ref { (LResourceRef*)listener };
    ref->setResource(nullptr);
}

void LFrameCallbacks::sendDoneAndDestroyFrames() noexcept
{
    const auto ms { CZTime::Ms() };

    while (!resources.empty())
    {
        if (resources.front().res())
        {
            wl_callback_send_done(resources.front().res(), ms);
            wl_resource_destroy(resources.front().res());
        }
        resources.pop_front();
    }
}
