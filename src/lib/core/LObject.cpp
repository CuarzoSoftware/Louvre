#include <LObject.h>
#include <LWeak.h>

using namespace Louvre;

LObject::~LObject() noexcept
{
    notifyDestruction();
}

void LObject::notifyDestruction() noexcept
{
    if (m_destroyed)
        return;

    m_destroyed = true;

    while (!m_weakRefs.empty())
    {
        LWeak<LObject> *weak { (LWeak<LObject>*)m_weakRefs.back() };
        weak->m_object = nullptr;
        m_weakRefs.pop_back();

        if (weak->m_onDestroyCallback)
            (*weak->m_onDestroyCallback)(this);
    }
}
