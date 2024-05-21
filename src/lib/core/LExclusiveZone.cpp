#include <private/LOutputPrivate.h>
#include <LLayerRole.h>
#include <LExclusiveZone.h>
#include <cassert>

using namespace Louvre;

LExclusiveZone::LExclusiveZone(LEdge edge, Int32 size, LOutput *output) noexcept
{
    m_output.setOnDestroyCallback([this](auto)
    {
        setOutput(nullptr);
    });

    setEdge(edge);
    setSize(size);
    setOutput(output);
}

LExclusiveZone::~LExclusiveZone()
{
    setOutput(nullptr);
}

void LExclusiveZone::setEdgeAndSize(LEdge edge, Int32 size) noexcept
{
    bool needsUpdate { false };

    if (size != m_size)
    {
        m_size = size;
        needsUpdate = true;
    }

    if (edge != m_edge)
    {
        assert((edge == 0 || edge == LEdgeLeft || edge == LEdgeTop || edge == LEdgeRight || edge == LEdgeBottom) && "Invalid edge passed to LExclusiveZone constructor.");
        m_edge = edge;
        needsUpdate = true;
    }

    if (needsUpdate)
        update();
}

void LExclusiveZone::setEdge(LEdge edge) noexcept
{
    if (edge == m_edge)
        return;

    assert((edge == 0 || edge == LEdgeLeft || edge == LEdgeTop || edge == LEdgeRight || edge == LEdgeBottom) && "Invalid edge passed to LExclusiveZone constructor.");
    m_edge = edge;
    update();
}

void LExclusiveZone::setSize(Int32 size) noexcept
{
    if (size == m_size)
        return;

    m_size = size;
    update();
}

void LExclusiveZone::setOutput(LOutput *output) noexcept
{
    if (output == m_output)
        return;

    if (m_output)
    {
        m_output->imp()->exclusiveZones.erase(m_outputLink);
        m_output->imp()->updateExclusiveZones();
    }

    m_output.reset(output);

    if (output)
    {
        output->imp()->exclusiveZones.emplace_back(this);
        m_outputLink = std::prev(output->imp()->exclusiveZones.end());
    }

    update();
}

LExclusiveZone::LExclusiveZone(LEdge edge, Int32 size, LLayerRole *layerRole, LOutput *output) noexcept
{
    m_layerRole.reset(layerRole);
    m_output.setOnDestroyCallback([this](auto)
    {
        setOutput(nullptr);
    });

    setEdge(edge);
    setSize(size);
    setOutput(output);
}

void LExclusiveZone::update() noexcept
{
    if (output())
        output()->imp()->updateExclusiveZones();
    else
        m_rect = LRect();
}
