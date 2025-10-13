#include <CZ/Louvre/Backends/Wayland/LWaylandOutputMode.h>
#include <CZ/Louvre/Backends/Wayland/LWaylandOutput.h>

using namespace CZ;

LOutput *LWaylandOutputMode::output() const noexcept
{
    if (m_output)
        return m_output->output();

    return {};
}
