#ifndef LCLIENTCURSOR_H
#define LCLIENTCURSOR_H

#include <LObject.h>
#include <CZ/skia/core/SkPoint.h>
#include <LCursorRole.h>
#include <LPointerEnterEvent.h>
#include <LSurface.h>

/**
 * @brief Encapsulates parameters used in a set cursor request.
 *
 * The LClientCursor class encapsulates the parameters requested by the client through LPointer::setCursorRequest().\n
 * You can retrieve the last cursor request for a specific client using LClient::lastCursorRequest().
 *
 * Clients create an LCursorRole for the compositor to use as the cursor. This class holds a reference to the role along with the LPointerEnterEvent that triggered the request.\n
 * Additionally, it includes a property indicating whether the client intends to hide the cursor or not.\n
 * This information gets overridden when the same client makes another LPointer::setCursorRequest() or when the associated LCursorRole is destroyed.
 *
 * When assigned to an LCursor through LCursor::setCursor(), the LCursor's texture and hotspot automatically change based on the values
 * of the associated LCursorRole. However, the cursor size remains user-defined.\n
 * If the LCursorRole surface is unmapped or destroyed after being assigned to an LCursor, the default cursor is automatically restored.
 */
class Louvre::LClientCursor final : public LObject
{
public:

    LCLASS_NO_COPY(LClientCursor)

    /**
     * @brief Indicates if the client wants to hide the cursor.
     *
     * Returns `true` if the client has never made a set cursor request.
     * If the LCursorRole is destroyed and this method returns `true`, and one of
     * the client surfaces has pointer focus, then the default cursor should be restored.
     *
     * @return `true` if the client intends to hide the cursor, `false` otherwise.
     */
    bool visible() const noexcept
    {
        return m_visible;
    }

    /**
     * @brief Returns the pointer enter event sent to the client that initiated the cursor request.
     *
     * @return The triggering LPointerEnterEvent.
     */
    const LPointerEnterEvent &triggeringEvent() const noexcept
    {
        return m_triggeringEvent;
    }

    /**
     * @brief Returns the LCursorRole the client used for this cursor.
     *
     * If the user has never made a set cursor request or the LCursorRole was destroyed, this returns `nullptr`.
     *
     * @return The associated LCursorRole or `nullptr` if no set cursor request has been made or the LCursorRole is destroyed.
     */
    LCursorRole *cursorRole() const noexcept
    {
        return m_role;
    }

    /**
     * @brief Returns the client owning the cursor.
     *
     * @return The associated LClient.
     */
    LClient *client() const noexcept
    {
        return m_client;
    }

private:
    friend class LClient;
    friend class LCursorRole;
    friend class Protocols::Wayland::RPointer;
    LClientCursor(LClient *client) noexcept : m_client(client) {}
    ~LClientCursor() noexcept;
    CZWeak<LCursorRole> m_role;
    LPointerEnterEvent m_triggeringEvent;
    LClient *m_client;
    bool m_visible { true };
};

#endif // LCLIENTCURSOR_H
