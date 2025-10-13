#ifndef LCURSOR_H
#define LCURSOR_H

#include <CZ/Ream/RImage.h>
#include <CZ/Louvre/LObject.h>
#include <CZ/Core/CZWeak.h>
#include <CZ/Core/CZCursorShape.h>
#include <CZ/skia/core/SkRect.h>
#include <memory>

/**
 * @brief Single cursor instance.
 *
 * Louvre creates a single instance of this class to display the cursor across all outputs.
 * This class also takes advantage of overlay cursor planes that may be available on outputs.
 * When cursor planes are not available or are disabled, the cursor should be painted manually.
 */
class CZ::LCursor : public LObject
{
public:

    /// Array of cursor sources for different shapes.
    using ShapeAssets = std::array<std::shared_ptr<LImageCursorSource>, size_t(CZCursorShape::MoveOrResize)>;

    /**
     * @brief Sets the cursor image and hotspot from the given source.
     *
     * If `nullptr` is passed, the fallback cursor is set instead.
     *
     * @param source The cursor source to use, or nullptr to use the fallback.
     */
    void setSource(std::shared_ptr<LCursorSource> source) noexcept;

    /**
     * @brief Returns the current cursor image and hotspot.
     *
     * Always returns a valid source. Initially, this is set to Louvre's default fallback cursor.
     */
    std::shared_ptr<LCursorSource> source() const noexcept { return m_source; }

    /**
     * @brief Returns the current fallback source.
     *
     * Used when `setSource(nullptr)` is called or when an asset for a specific shape is not found.
     *
     * Initially, this is set to Louvre's default fallback cursor.
     *
     * @return The fallback cursor source. Always valid.
     */
    std::shared_ptr<LImageCursorSource> fallbackSource() const noexcept { return m_fallbackSource; }

    /**
     * @brief Replaces the fallback cursor source.
     *
     * If `nullptr` is passed, the default fallback source provided by Louvre is set.
     *
     * @param source The new fallback cursor source, or nullptr to reset to the default.
     */
    void setFallbackSource(std::shared_ptr<LImageCursorSource> source) noexcept;

    /**
     * @brief Stores or unsets a cursor source for a given shape.
     *
     * When an LShapeCursorSource source is set, LCursor looks for an existing asset or uses the fallback source.
     * Assets are initially empty and should be populated with your own sources.
     */
    void setShapeAsset(CZCursorShape shape, std::shared_ptr<LImageCursorSource> source) noexcept;
    std::shared_ptr<LImageCursorSource> getShapeAsset(CZCursorShape shape) const noexcept { return m_shapeAssets[(size_t)shape - 1]; }

    /**
     * @brief Sets the cursor position in compositor-global coordinates.
     *
     * @note Louvre automatically repositions the cursor if the specified location is not within any output.
     */
    void setPos(SkPoint pos) noexcept;
    void setPos(Float32 x, Float32 y) noexcept { setPos(SkPoint(x, y)); }

    /**
     * @brief Adjusts the cursor position by a delta (dx, dy).
     */
    void move(SkPoint delta) noexcept { setPos(pos() + delta); }
    void move(Float32 dx, Float32 dy) noexcept { move(SkPoint(dx, dy)); };

    /**
     * @brief Gets the current cursor position in compositor-global coordinates.
     */
    SkPoint pos() const noexcept { return m_pos; }

    /**
     * @brief Gets the cursor rect on the screen.
     *
     * Returns the cursor rect, which is defined as SkIRect(pos - hotspot, size), in compositor-global coordinates.
     *
     * You should use this rect and the current source to paint the cursor when a plane is not available.
     */
    const SkIRect &rect() const noexcept { return m_rect; };

    /**
     * @brief Sets the cursor size.
     *
     * Sets the cursor size in surface coordinates. The source image and hotspot are automatically scaled, see rect().
     *
     * The initial cursor size is (24, 24).
     */
    void setSize(SkSize size) noexcept;
    void setSize(Float32 width, Float32 height) noexcept { setSize(SkSize(width, height)); };
    SkSize size() const noexcept { return m_size; }

    /**
     * @brief Toggles the cursor visibility.
     */
    void setVisible(bool visible) noexcept;

    /**
     * @brief Checks if the cursor is visible
     */
    bool isVisible() const noexcept { return m_isVisible; };

    /**
     * @brief Gets the current cursor output.
     *
     * Returns the output where the cursor is currently positioned.
     *
     * @note This method always returns a valid output except when none is initialized.
     */
    LOutput *output() const noexcept;

    /**
     * @brief Set of intersected outputs.
     *
     * Returns a vector of initialized outputs that intersect the cursor's rect().
     */
    const std::unordered_set<LOutput*> &intersectedOutputs() const noexcept { return m_intersectedOutputs; };

    /**
     * @brief Invokes LOutput::repaint() for each intersected outputs.
     *
     * @param softwareOnly If `true`, only repaints outputs that don't have a cursor plane.
     */
    void repaintOutputs(bool softwareOnly = true) noexcept;

    /**
     * @brief Checks if a given output has a cursor plane.
     */
    bool hasPlane(const LOutput *output) const noexcept;

    /**
     * @brief Toggles the cursor plane for the specified output.
     *
     * When disabled, the cursor should be painted manually (see rect()).
     *
     * The cursor plane is enabled by default if the output has one.
     *
     * @param output The output for which to enable or disable hardware compositing.
     * @param enabled Set to `true` to enable hardware compositing, or `false` to disable it.
     */
    void enablePlane(LOutput *output, bool enabled) noexcept;

    /**
     * @brief Checks if the cursor plane is enabled for the specified output.
     *
     * @note This method always returns `false` if hasPlane() for the given output returns `false`.
     */
    bool isPlaneEnabled(const LOutput *output) const noexcept;

    std::shared_ptr<const RSurface> surface() const noexcept { return m_surface; }
    ~LCursor() noexcept;

private:
    friend class CZ::LCompositor;
    friend class CZ::LOutput;
    LCursor() noexcept;
    void setOutput(LOutput *output) noexcept;
    void updateLater() const noexcept;
    void update() noexcept;

    SkPoint m_pos {};
    SkSize m_size {};
    SkIPoint m_hotspot {};
    SkIRect m_rect {};
    std::shared_ptr<RImage> m_image;
    std::shared_ptr<LCursorSource> m_source;
    std::shared_ptr<LImageCursorSource> m_fallbackSource;
    std::shared_ptr<LImageCursorSource> m_louvreSource;

    UInt32 m_imageWriteSerial { 0 };
    mutable CZWeak<LOutput> m_output;
    std::unordered_set<LOutput*> m_intersectedOutputs;

    bool m_isVisible { true };
    mutable bool m_posChanged { false };
    mutable bool m_imageChanged { false };

    ShapeAssets m_shapeAssets;

    std::shared_ptr<RSurface> m_surface;
    UInt8 m_buffer[64*64*4];
};

#endif // LCURSOR_H
