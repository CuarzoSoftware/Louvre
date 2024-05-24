#ifndef LEXCLUSIVEZONE_H
#define LEXCLUSIVEZONE_H

#include <LObject.h>
#include <LWeak.h>
#include <LRect.h>
#include <functional>
#include <list>

class Louvre::LExclusiveZone : public LObject
{
public:

    /**
     * Callback function type used to handle the `onRectChange()` event.
     */
    using OnRectChangeCallback = std::function<void(LExclusiveZone*)>;

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

    bool insertAfter(LExclusiveZone *zone) const;
    LExclusiveZone *nextZone() const noexcept;
    LExclusiveZone *prevZone() const noexcept;

    // Rect describing the exclusive zone in output-local surface coordinates
    // If the output is nullptr the rect is (0,0,0,0)
    const LRect &rect() const noexcept
    {
        return m_rect;
    }

    void setOnRectChangeCallback(const OnRectChangeCallback &callback)
    {
        m_onRectChangeCallback = callback;
    }

    const OnRectChangeCallback &onRectChangeCallback() const noexcept
    {
        return m_onRectChangeCallback;
    }

private:
    friend class LOutput;
    void update() noexcept;
    LWeak<LOutput> m_output;
    LEdge m_edge;
    Int32 m_size;
    LRect m_rect;
    mutable std::list<LExclusiveZone*>::iterator m_outputLink;
    OnRectChangeCallback m_onRectChangeCallback { nullptr };
};

#endif // LEXCLUSIVEZONE_H
