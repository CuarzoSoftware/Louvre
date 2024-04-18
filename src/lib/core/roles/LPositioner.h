#ifndef LPOSITIONER_H
#define LPOSITIONER_H

#include <LObject.h>
#include <LRect.h>
#include <LBitset.h>
#include <memory>

/*!
 * @brief Positioning rules for a Popup
 *
 * The LPositioner class defines the rules by which a Popup should be positioned relative to the anchor point of its parent.\n
 * Each LPopupRole has its own LPositioner accessible through LPopupRole::positioner().\n
 *
 * @note Louvre implements the default LPositioner rules in LPopupRole::rolePos(), therefore this section is for educational purposes only.
 *
 * @section Anchor-Rect
 *
 * The anchor rect (accessible through anchorRect()) is a sub-rect of the window geometry of the Popup's parent surface.\n
 * Within this rect, an anchor point (accessible through anchor()) is defined, which the Popup uses as a reference to position itself.\n
 * The anchor point can be located at the center, corners, or midpoint of the edges of the anchor rect (gray and blue points).\n
 * For example, in the following image, a Popup is positioned using the anchor point LPositioner::AnchorRight and LPositioner::AnchorBottomLeft.\n
 *
 * <IMG SRC="https://lh3.googleusercontent.com/dCpt0Kl2MBnwpxf7VUiphJ28Tdrxh-3jmNIG-9GyK9nt_-3vCMuj1vgmYajPnYd9CH51fIBYocCUlBKsdXAGqQnufFxYZ1whQ0T6pIfCO1E6NHJj-ii2phQY-kRdUe2lZAnqF0mvyA=w2400">
 *
 * @section Gravity
 *
 * The gravity of the Popup, accessible through gravity(), indicates the direction that the Popup tries to move to.\n
 * You can consider the anchor point as a "nail" and the Popup as a frame composed only of edges. If the gravity is down,
 * the top edge of the Popup will collide with the nail, preventing it from moving further.\n
 * In the following image, a Popup with gravity LPositioner::GravityBottomRight and LPositioner::GravityTopLeft is shown.\n
 *
 * <IMG SRC="https://lh3.googleusercontent.com/92DINcYGHOAothPGrLctUtN7mCKRpqESPh4vRA8XN--IehoprgWKYn74myk1CsjXMR_IcaLM7kJZWny7rvytDRr-nXzljMA-W0LWtQ4neu-HGxpT8V2P0blWg5zYymbGQ8vja5Rx6w=w2400">
 *
 * @section Constraint-Adjustments
 *
 * You might be wondering why use such complex rules to position Popups? The answer is that they make it easier to reposition them in cases when their position causes them to be constrained (for example, when they are shown outside the visible area of the screen).\n
 * Specific rules to adjust the position of Popups when they are constrained are defined by the client in constraintAdjustment().\n
 * Typically, it first tries to invert the gravity and anchor point on the axes where the constraint occurs (for example, in nested context menus), if the Popup is still constrained, it tries to slide it along the relevant axes,
 * if this still doesn't solve the problem, the Popup is configured to adjust its size and if finally none of these options work, the original position is used.\n
 * For more information on LPositioner rules, you can analyze the implementation of LPopupRole::rolePos() or consult the documentation of the [xdg_shell::positioner](https://wayland.app/protocols/xdg-shell#xdg_positioner) interface.
 */
class Louvre::LPositioner
{
public:
    /// @cond OMIT
    LPositioner() noexcept = default;
    ~LPositioner() noexcept = default;
    /// @endcond

    /*!
     * @brief Anchor point.
     *
     * Possible locations of the anchor point within the anchor rect.
     */
    enum Anchor : UInt32
    {
        /// Center of the anchor rect
        NoAnchor = 0,

        /// Center of the top edge
        AnchorTop         = 1,

        /// Center of the bottom edge
        AnchorBottom      = 2,

        /// Center of the left edge
        AnchorLeft        = 3,

        /// Center of the right edge
        AnchorRight       = 4,

        /// Top-left corner
        AnchorTopLeft     = 5,

        /// Bottom-left corner
        AnchorBottomLeft  = 6,

        /// Top-right corner
        AnchorTopRight    = 7,

        /// Bottom-right corner
        AnchorBottomRight = 8
    };

    /*!
     * @brief Popup gravity.
     *
     * The direction in which the Popup tries to move.
     */
    enum Gravity : UInt32
    {
        /// Centered
        NoGravity = 0,

        /// Upwards
        GravityTop         = 1,

        /// Downwards
        GravityBottom      = 2,

        /// Leftwards
        GravityLeft        = 3,

        /// Rightwards
        GravityRight       = 4,

        /// Upwards and Leftwards
        GravityTopLeft     = 5,

        /// Downwards and Leftwards
        GravityBottomLeft  = 6,

        /// Upwards and Rightwards
        GravityTopRight    = 7,

        /// Downwards and Rightwards
        GravityBottomRight = 8
    };

    /*!
     * @brief Constraint adjustments.
     *
     * Rules for handling a constrained Popup.
     */
    enum ConstraintAdjustments : UInt32
    {
        /// Original position is not modified
        NoAdjustment = 0,

        /// Horizontally slide
        SlideX          = 1,

        /// Vertically slide
        SlideY          = 2,

        /// Invert horizontal component of gravity and anchor point
        FlipX           = 4,

        /// Invert vertical component of gravity and anchor point
        FlipY           = 8,

        /// Scale Popup horizontally
        ResizeX         = 16,

        /// Scale Popup vertically
        ResizeY         = 32
    };

    /*!
     * @brief Size in surface coordinates.
     *
     * Size of the Popup to be positioned (window geometry) in surface coordinates.
     */
    const LSize &size() const
    {
        return m_size;
    }

    /*!
     * @brief Anchor rect in surface coordinates.
     *
     * Anchor rect relative to the parent's geometry origin in surface coordinates.
     */
    const LRect &anchorRect() const
    {
        return m_anchorRect;
    }

    /*!
     * @brief Additional offset in surface coordinates.
     *
     * Additional offset in surface coordinates added to the Popup's position calculated by the rules.
     */
    const LPoint &offset() const noexcept
    {
        return m_offset;
    }

    /*!
     * @brief Anchor point.
     *
     * Point on the anchor rect defined by LPositioner::Anchor enum.
     */
    Anchor anchor() const noexcept
    {
        return m_anchor;
    }

    /*!
     * @brief Popup gravity.
     *
     * Direction in which the Popup is trying to move, defined in LPositioner::Gravity.
     */
    Gravity gravity() const noexcept
    {
        return m_gravity;
    }

    /*!
     * @brief Constraint adjustment rules.
     *
     * Flags with the rules to use in case the Popup is constrained, defined in LPositioner::ConstraintAdjustment.
     */
    LBitset<ConstraintAdjustments> constraintAdjustments() const noexcept
    {
        return m_constraintAdjustments;
    }

private:
    friend class LPopupRole;
    friend class Protocols::XdgShell::RXdgPositioner;
    LSize m_size;
    LRect m_anchorRect;
    LPoint m_offset;

    Anchor m_anchor { Anchor::NoAnchor };
    Gravity m_gravity { Gravity::NoGravity };
    LBitset<ConstraintAdjustments> m_constraintAdjustments { ConstraintAdjustments::NoAdjustment };

    // Since 3
    bool isReactive { false };
    LSize parentSize;
    UInt32 parentConfigureSerial;
};

#endif // LPOSITIONER_H
