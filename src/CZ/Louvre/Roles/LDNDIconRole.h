#ifndef LDNDICONROLE_H
#define LDNDICONROLE_H

#include <CZ/Louvre/Roles/LBaseSurfaceRole.h>

/**
 * @brief Drag & drop icon role for surfaces
 *
 * The LDNDIconRole role is used during drag & drop sessions. See LDND::icon().
 *
 * <center><IMG WIDTH="250px" SRC="https://lh3.googleusercontent.com/evKJ2MbTJ42-qFYSP02NPxUULSFpTz3oBSqn6RvR20u_r5wvgJpHF6o-3Zg7aWgNBhrkIsM8iNWiQQHxPjvGml9zDB2wwNwWK0scqTsHpLIbxMqYv60afSruzbWNBCDZaGI_y77eRA=w2400"></center>
 */

class CZ::LDNDIconRole : public LBaseSurfaceRole
{
public:

    struct Params;

    static constexpr LFactoryObject::Type FactoryObjectType = LFactoryObject::Type::LDNDIconRole;

    /**
     * @brief Constructor of the LDNDIconRole class.
     *
     * @param params Internal parameters provided in LCompositor::createObjectRequest().
     */
    LDNDIconRole(const void *params) noexcept;

    /**
     * @brief Destructor of the LDNDIconRole class.
     *
     * Invoked after LCompositor::onAnticipatedObjectDestruction().
     *
     * @warning The `surface()` handle always remains valid during the destructor call.
     *          However, `LSurface::role()` returns `nullptr` because `LSurface::roleChanged()`
     *          is notified beforehand and requires the role to be valid.
     */
    ~LDNDIconRole() noexcept;

    /**
     * @brief Hotspot of the icon in surface coordinates.
     */
    SkIPoint hotspot() const noexcept { return m_hotspot; }

    /**
     * @brief Hotspot of the icon in buffer coordinates.
     */
    SkIPoint hotspotB() const noexcept { return m_hotspotB; }

    /**
     * @brief Notify a hotspot change.
     *
     * #### Default implementation
     * @snippet LDNDIconRoleDefault.cpp hotspotChanged
     */
    virtual void hotspotChanged() const;

    /**
     * @brief Position of the surface given the role.
     *
     * The position of the icon given the role is calculated by subtracting the hotspot from LSurface::pos().
     *
     * #### Default implementation
     * @snippet LDNDIconRoleDefault.cpp rolePos
     */
    virtual SkIPoint rolePos() const override;

private:
    struct State
    {
        SkIPoint offset;
    };

    void handleSurfaceOffset(Int32 x, Int32 y) override;
    void cacheCommit() noexcept override;
    void applyCommit() noexcept override;

    SkIPoint m_hotspotB {};
    SkIPoint m_hotspot {};

    State m_current {};
    State m_pending {};
    std::list<State> m_cache;
};

#endif // LDNDICONROLE_H
