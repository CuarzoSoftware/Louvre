#ifndef LEXCLUSIVEZONE_H
#define LEXCLUSIVEZONE_H

#include <LObject.h>
#include <LWeak.h>
#include <LRect.h>
#include <list>

class Louvre::LExclusiveZone : public LObject
{
public:
    LExclusiveZone(LEdge edge, Int32 size, LOutput *output = nullptr) noexcept;
    ~LExclusiveZone();

    void setEdgeAndSize(LEdge edge, Int32 size) noexcept;
    void setEdge(LEdge edge) noexcept;
    void setSize(Int32 size) noexcept;
    void setOutput(LOutput *output) noexcept;

    LOutput *output() const noexcept
    {
        return m_output.get();
    }

    LEdge edge() const noexcept
    {
        return m_edge;
    }

    Int32 size() const noexcept
    {
        return m_size;
    }

    // Rect describing the exclusive zone in output-local surface coordinates
    // If the output is nullptr the rect is (0,0,0,0)
    const LRect &rect() const noexcept
    {
        return m_rect;
    }

private:
    LExclusiveZone(LEdge edge, Int32 size, LLayerRole *layerRole, LOutput *output = nullptr) noexcept;
    friend class LOutput;
    friend class LLayerRole;
    void update() noexcept;
    LWeak<LOutput> m_output;
    LWeak<LLayerRole> m_layerRole;
    LEdge m_edge;
    Int32 m_size;
    LRect m_rect;
    std::list<LExclusiveZone*>::iterator m_outputLink;
};

#endif // LEXCLUSIVEZONE_H
