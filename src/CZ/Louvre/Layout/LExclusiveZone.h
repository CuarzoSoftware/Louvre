#ifndef LEXCLUSIVEZONE_H
#define LEXCLUSIVEZONE_H

#include <CZ/Louvre/LObject.h>
#include <CZ/Core/CZWeak.h>
#include <CZ/skia/core/SkRect.h>
#include <CZ/Core/CZEdge.h>
#include <functional>
#include <list>

/**
 * @brief Exclusive zone within an LOutput
 *
 * @anchor lexclusivezone_detailed
 *
 * The LExclusiveZone class allows you to define a specific region relative to an LOutput edge to be considered exclusive.\n
 * It is primarily used by LLayerRole surfaces to prevent other elements from occupying their space, but it can also
 * be utilized by custom compositor UI elements, such as a built-in panel.
 *
 * When assigned to an output, exclusive zones affect LOutput::availableGeometry() and LOutput::exclusiveEdges().
 * These properties are used, for example, to properly position and resize LToplevelRole surfaces.
 *
 * To define an exclusive zone, you need to specify the output(), the edge() it is anchored to, and the distance (size()) from that edge.\n
 * The rect() property will be updated to indicate the position and size of the exclusive zone within the output.
 *
 * Multiple exclusive zones can be assigned to the same output. In such cases, the zones located at the beginning of the LOutput::exclusiveZones()
 * list have preference, causing other zones to be moved/resized to prevent occlusion of the predominant ones.
 *
 * @note The order of the list can be modified using insertAfter().
 *
 * The list order also determines the arrangement of multiple exclusive zones attached to the same edge. For example, if two exclusive zones are
 * anchored to the top edge, the first one will be on top, and the second one will be closer to the center of the output.
 *
 * You can also specify a callback function to be notified when an exclusive zone's rect() changes using the setOnRectChangeCallback() method.
 */
class CZ::LExclusiveZone final : public LObject
{
public:

    /**
     * Callback function type used to handle the `onRectChange()` event.
     */
    using OnRectChangeCallback = std::function<void(LExclusiveZone*)>;

    /**
     * @brief Constructor for LExclusiveZone
     *
     * Initializes an exclusive zone with the specified edge, size, and output.
     *
     * @param edge The edge of the output to anchor the exclusive zone to.
     * @param size The distance from the edge to define the exclusive zone.
     * @param output The output to assign the exclusive zone to.
     */
    LExclusiveZone(CZEdge edge, Int32 size, LOutput *output = nullptr) noexcept;

    /**
     * @brief Destructor for LExclusiveZone
     */
    ~LExclusiveZone();

    /**
     * @brief Sets the edge and size simultaneously
     *
     * Each time the edge and/or size change, the rect() of all zones within the same output are recalculated.\n
     * This method allows you to change both parameters at once with a single update, which is more efficient.
     *
     * @param edge The edge of the output to anchor the exclusive zone to.
     * @param size The distance from the edge to define the exclusive zone.
     */
    void setEdgeAndSize(CZEdge edge, Int32 size) noexcept;

    /**
     * @brief Sets the edge of the output to which the exclusive zone is anchored.
     *
     * Accepts a single @ref CZEdge or @ref CZEdgeNone .\n
     * If set to @ref CZEdgeNone, the zone will not affect others, regardless of the value of size().
     *
     * - If set to @ref CZEdgeNone and size() is less than 0, rect() will occupy the full output size.
     * - If set to @ref CZEdgeNone and size() is greater than or equal to 0, rect() will always be equal to LOutput::availableGeometry().
     *
     * @param edge The edge of the output to anchor the exclusive zone to.
     */
    void setEdge(CZEdge edge) noexcept;

    /**
     * @brief Sets the size of the exclusive zone.
     *
     * - If the size is less than 0, the zone will not interact with others. In this case, rect() will be positioned at the given
     *   edge with a size of 0 and will expand to the orthogonal edges, or occupy the full output size if @ref CZEdgeNone is set.
     * - If the size is 0, the zone will be affected (moved/resized) by other zones, but it will not affect others.
     *
     * @see setEdge()
     *
     * @param size The distance from the edge to define the exclusive zone.
     */
    void setSize(Int32 size) noexcept;

    /**
     * @brief Sets the current output.
     *
     * The zone is added at the end of the LOutput::exclusiveZones() list.
     *
     * @param output The output to assign the exclusive zone to, or `nullptr` to remove it from all outputs.
     */
    void setOutput(LOutput *output) noexcept;

    /**
     * @brief Gets the current output.
     *
     * @return The current output, or `nullptr` if not assigned.
     */
    LOutput *output() const noexcept
    {
        return m_output.get();
    }

    /**
     * @brief Gets the current edge.
     *
     * The edge the exclusive zone is anchored to, set with setEdge().
     *
     * @return The current edge.
     */
    CZEdge edge() const noexcept
    {
        return m_edge;
    }

    /**
     * @brief Gets the exclusive zone size.
     *
     * The size of the exclusive zone, set with setSize().
     *
     * @return The size of the exclusive zone.
     */
    Int32 size() const noexcept
    {
        return m_size;
    }

    /**
     * @brief Inserts this zone after the given zone in the LOutput::exclusiveZones() list.
     *
     * This operation is only successful if both exclusive zones belong to the same output.
     *
     * @param zone The zone after which this zone should be inserted.
     * @return `true` if successful, `false` on failure.
     */
    bool insertAfter(LExclusiveZone *zone) const;

    /**
     * @brief Next exclusive zone in the LOutput::exclusiveZones() list.
     *
     * @return The next exclusive zone, or `nullptr` if no output is assigned or it is the last exclusive zone.
     */
    LExclusiveZone *nextZone() const noexcept;

    /**
     * @brief Previous exclusive zone in the LOutput::exclusiveZones() list.
     *
     * @return The previous exclusive zone, or `nullptr` if no output is assigned or it is the first exclusive zone.
     */
    LExclusiveZone *prevZone() const noexcept;

    /**
     * @brief Gets the calculated position and size of the exclusive zone in output-local surface coordinates.
     *
     * @note If the output is `nullptr`, the rect is (0, 0, 0, 0).
     *
     * @return The rect representing the position and size of the exclusive zone.
     */
    const SkIRect &rect() const noexcept
    {
        return m_rect;
    }

    /**
     * @brief Sets the rect change listener.
     *
     * This callback is triggered each time the exclusive zone rect() changes, for example, after modifying its parameters or when other zones change.
     *
     * @param callback The callback function to be called on rect changes.
     */
    void setOnRectChangeCallback(const OnRectChangeCallback &callback) noexcept
    {
        m_onRectChangeCallback = callback;
    }

    /**
     * @brief Gets the callback set for rect changes.
     *
     * @return The callback function set with setOnRectChangeCallback().
     */
    const OnRectChangeCallback &onRectChangeCallback() const noexcept
    {
        return m_onRectChangeCallback;
    }

private:
    friend class LOutput;
    void update() noexcept;
    CZWeak<LOutput> m_output;
    CZEdge m_edge;
    Int32 m_size { 0 };
    SkIRect m_rect { 0, 0, 0, 0 };
    mutable std::list<LExclusiveZone*>::iterator m_outputLink;
    OnRectChangeCallback m_onRectChangeCallback { nullptr };
};

#endif // LEXCLUSIVEZONE_H
