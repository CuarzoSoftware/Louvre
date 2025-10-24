#include <CZ/Louvre/LGlobal.h>
#include <CZ/Louvre/LLog.h>

using namespace CZ;

const wl_interface *LGlobal::interface() const noexcept
{
    return wl_global_get_interface(m_global);
}

LGlobal::LGlobal(wl_global *global) noexcept : m_global(global)
{
    LLog(CZDebug, "[LGlobal] {} created", interface()->name);
}

LGlobal::~LGlobal() noexcept
{
    notifyDestruction();
    LLog(CZDebug, "[LGlobal] {} destroyed", interface()->name);
    wl_global_destroy(m_global);
}
