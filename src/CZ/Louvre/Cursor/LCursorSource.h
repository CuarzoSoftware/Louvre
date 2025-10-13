#ifndef CZ_LCURSORSOURCE_H
#define CZ_LCURSORSOURCE_H

#include "Core/CZCursorShape.h"
#include <CZ/Louvre/LObject.h>
#include <CZ/Ream/Ream.h>
#include <memory>

/**
 * @brief A cursor source for LCursor.
 */
class CZ::LCursorSource : public LObject
{
public:
    /**
     * @brief Types of cursor sources.
     */
    enum Type
    {
        /// Created from a user-supplied image
        Image,

        /// Created after an LCursorRole
        Role,

        /// Cursor referencing one of the predefined LCursor shapes (cursor shape protocol)
        Shape
    };

    /**
     * @brief Cursor visibility modes.
     *
     * Defines how a source affects the LCursor visibility when applied.
     * Used by cursor sources from clients that may carry visibility information.
     */
    enum Visibility
    {
        /// The cursor is hidden when applied, regardless of the LCursor state.
        Hidden,

        /// The cursor is made visible when applied, regardless of the LCursor state.
        Visible,

        /// The cursor preserves the current LCursor visibility state when applied.
        Auto
    };

    /**
     * @brief The cursor type.
     */
    Type type() const noexcept { return m_type; }

    /**
     * @brief Create a cursor from a user-supplied image.
     *
     * @param image   The cursor image. Must be valid.
     * @param hotspot The hotspot position in buffer coordinates.
     * @return A shared pointer to the created cursor source, or nullptr if the image is invalid.
     */
    static std::shared_ptr<LImageCursorSource> Make(std::shared_ptr<RImage> image, SkIPoint hotspot) noexcept;

    /**
     * @brief Load a cursor from an XCursor theme.
     *
     * @param name           The name of the cursor to load (must not be nullptr).
     * @param theme          The XCursor theme name, or nullptr to use the default theme.
     * @param suggestedSize  The preferred size of the cursor in pixels (default: 64).
     *
     * @return A shared pointer to the loaded cursor image, or nullptr on failure.
     */
    static std::shared_ptr<LImageCursorSource> MakeFromTheme(const char *name, const char *theme = nullptr, Int32 suggestedSize = 64) noexcept;

    /**
     * @brief Create a cursor from a shape.
     *
     * When assigned to LCursor, the image and hotspot will reference one of the stored shape assets.
     * If no asset exists for the given shape, the fallback source is used instead.
     */
    static std::shared_ptr<LShapeCursorSource> MakeShape(CZCursorShape shape) noexcept;

    /**
     * @brief Get the cursor image.
     *
     * @return A valid shared pointer to the cursor image.
     */
    virtual std::shared_ptr<RImage> image() const noexcept = 0;

    /**
     * @brief Get the hotspot location.
     *
     * @return The hotspot in buffer coordinates.
     */
    virtual SkIPoint hotspot() const noexcept = 0;

    /**
     * @brief Gets the visibility mode of the cursor.
     *
     * @return The current cursor visibility mode.
     */
    virtual Visibility visibility() const noexcept { return Auto; }

    /**
     * @brief Get the client that created this cursor.
     *
     * @return Pointer to the client, or nullptr if the cursor was not created by a client.
     */
    virtual LClient *client() const noexcept { return nullptr; }

    /**
     * @brief Get the triggering event that requested the cursor change.
     *
     * @return Pointer to the event, or nullptr if the cursor was not created by a client.
     */
    virtual const CZEvent *triggeringEvent() const noexcept { return nullptr; }

    // Cast
    std::shared_ptr<LImageCursorSource> asImage() noexcept;
    std::shared_ptr<LRoleCursorSource> asRole() noexcept;
    std::shared_ptr<LShapeCursorSource> asShape() noexcept;

protected:
    friend class LCursor;
    std::weak_ptr<LCursorSource> m_self;

    /**
     * @brief Called when the cursor enters an output.
     */
    virtual void onEnter(LOutput *output) noexcept { CZ_UNUSED(output) };

    /**
     * @brief Called when the cursor leaves an output.
     */
    virtual void onLeave(LOutput *output) noexcept { CZ_UNUSED(output) };

    /**
     * @brief Construct a cursor source.
     */
    LCursorSource(Type type) noexcept : m_type(type) {};

private:
    Type m_type;
};

#endif // CZ_LCURSORSOURCE_H
