#include <LGlobal.h>
#include <LLog.h>

using namespace Louvre;

const wl_interface *LGlobal::interface() const noexcept
{
    return wl_global_get_interface(m_global);
}

LGlobal::LGlobal(wl_global *global) noexcept : m_global(global)
{
    LLog::debug("[LGlobal] %s created.", interface()->name);
}

LGlobal::~LGlobal() noexcept
{
    notifyDestruction();
    LLog::debug("[LGlobal] %s destroyed.", interface()->name);
    wl_global_destroy(m_global);
}
