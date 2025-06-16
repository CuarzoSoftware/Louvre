#ifndef LPOSITIONER_H
#define LPOSITIONER_H

#include <CZ/skia/core/SkRect.h>
#include <CZ/CZBitset.h>

/*!
 * @brief Positioning rules for LPopupRole surfaces.
 *
 * The LPositioner class defines the rules by which a popup should be positioned relative to the anchor point of its parent.\n
 * Each LPopupRole has its own LPositioner accessible through LPopupRole::positioner().\n
 *
 * @section positioner-anchor-rect Anchor Rect
 *
 * The anchorRect() defines a sub-rectangle within the parent surface's window geometry.
 * Within this sub-rectangle, an anchor point is specified, which the popup uses as a reference for positioning itself.
 *
 * The anchor point can be located at the center, the corners, or the midpoints of the edges of the anchor rectangle (indicated by gray and blue points).
 *
 * For example, in the following image, a popup is positioned using the anchor points @ref AnchorRight and @ref AnchorBottomLeft.
 *
 * <IMG SRC="https://lh3.googleusercontent.com/dCpt0Kl2MBnwpxf7VUiphJ28Tdrxh-3jmNIG-9GyK9nt_-3vCMuj1vgmYajPnYd9CH51fIBYocCUlBKsdXAGqQnufFxYZ1whQ0T6pIfCO1E6NHJj-ii2phQY-kRdUe2lZAnqF0mvyA=w2400">
 *
 * @section positioner-gravity Gravity
 *
* The gravity() of the popup indicates the direction in which it tries to move.
 *
 * You can visualize the anchor point as a "nail" and the popup as a frame composed only of edges. If the gravity is down,
 * the top edge of the popup will collide with the nail, preventing it from moving further.
 *
 * In the following image, a popup is shown with gravity set to @ref GravityBottomRight and @ref GravityTopLeft .
 *
 * <IMG SRC="https://lh3.googleusercontent.com/92DINcYGHOAothPGrLctUtN7mCKRpqESPh4vRA8XN--IehoprgWKYn74myk1CsjXMR_IcaLM7kJZWny7rvytDRr-nXzljMA-W0LWtQ4neu-HGxpT8V2P0blWg5zYymbGQ8vja5Rx6w=w2400">
 *
 * @section positioner-constraint-adjustments Constraint Adjustments
 *
 * You might be wondering why such complex rules are used to position popups. The answer is that these rules simplify repositioning
 * popups when their initial position causes constraints (e.g., when they appear outside the visible area of the screen).
 *
 * The specific rules for adjusting the position of popups when they are constrained are defined by the client in constraintAdjustments().
 * Typically, the process involves the following steps:
 * 1. Inverting the gravity and anchor point on the axes where the constraint occurs (useful in nested context menus).
 * 2. Sliding the popup along the relevant axes if it remains constrained.
 * 3. Adjusting the popup's size if sliding does not resolve the issue.
 * 4. Reverting to the original position if none of the above options work.
 *
 * For more information on LPositioner rules, consult the documentation of the [xdg_shell::positioner](https://wayland.app/protocols/xdg-shell#xdg_positioner) interface.
 */
class Louvre::LPositioner
{
public:

    /*!
     * @brief Anchor point.
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
     * @brief Gravity.
     *
     * The direction in which the popup tries to move.
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
     * Rules for handling a constrained popup.
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
     * Size of the popup to be positioned (window geometry size) in surface coordinates.
     */
    const SkISize &size() const
    {
        return m_size;
    }

    /*!
     * @brief Anchor rect in surface coordinates.
     *
     * Anchor rect relative to the parent window geometry in surface coordinates.
     */
    const SkIRect &anchorRect() const
    {
        return m_anchorRect;
    }

    /*!
     * @brief Additional offset in surface coordinates.
     *
     * Additional offset in surface coordinates added to the final calculated position.
     */
    const SkIPoint &offset() const noexcept
    {
        return m_offset;
    }

    /*!
     * @brief Anchor point.
     *
     * Edge or corner within the anchorRect() the popup is positioned relative to.
     */
    Anchor anchor() const noexcept
    {
        return m_anchor;
    }

    /*!
     * @brief Popup gravity.
     *
     * Direction in which the popup is trying to move, defined in LPositioner::Gravity.
     */
    Gravity gravity() const noexcept
    {
        return m_gravity;
    }

    /*!
     * @brief Constraint adjustment rules.
     *
     * Flags with rules used to unconstrain the popup.
     */
    CZBitset<ConstraintAdjustments> constraintAdjustments() const noexcept
    {
        return m_constraintAdjustments;
    }

    /**
     * @brief Determines if the popup should be reconfigured, for example, when the parent surface moves.
     *
     * This method checks whether the popup needs to be reconfigured
     * in response to changes in the parent surface, such as movement or other configuration changes.
     *
     * @return `true` if the popup should be reconfigured, `false` otherwise.
     */
    bool reactive() const noexcept
    {
        return m_reactive;
    }

    /**
     * @brief Checks if the popup is being repositioned according to a future parent size.
     *
     * The compositor may use parentConfigureSerial() together with parentSize() to determine what
     * future state the popup should be constrained using during an LPopupRole::configureRequest().
     *
     * @see parentConfigureSerial()
     * @see LPopupRole::configureRequest()
     *
     * @return `true` if the popup is being repositioned due to a parent configure change, `false` otherwise.
     */
    bool hasParentSize() const noexcept
    {
        return m_hasParentSize;
    }

    /**
     * @brief Parent size.
     *
     * @see hasParentSize().
     */
    const SkISize &parentSize() const noexcept
    {
        return m_parentSize;
    }

    /**
     * @brief Checks if the popup is being repositioned according to a future parent configuration.
     *
     * The compositor may use parentConfigureSerial() together with parentSize() to determine what
     * future state the popup should be constrained using during an LPopupRole::configureRequest().
     *
     * @see parentConfigureSerial()
     * @see LPopupRole::configureRequest()
     *
     * @return `true` if the popup is being repositioned according to a future parent configuration, `false` otherwise.
     */
    bool hasParentConfigureSerial() const noexcept
    {
        return m_hasParentConfigureSerial;
    }

    /**
     * @brief Parent configuration serial.
     *
     * @see hasParentConfigureSerial().
     */
    UInt32 parentConfigureSerial() const noexcept
    {
        return m_parentConfigureSerial;
    }

private:
    friend class LPopupRole;
    friend class Protocols::XdgShell::RXdgPositioner;

    SkISize m_size;
    SkIRect m_anchorRect;
    SkIPoint m_offset;

    Anchor m_anchor { Anchor::NoAnchor };
    Gravity m_gravity { Gravity::NoGravity };
    CZBitset<ConstraintAdjustments> m_constraintAdjustments { ConstraintAdjustments::NoAdjustment };

    // Since 3
    SkISize m_parentSize;
    UInt32 m_parentConfigureSerial;
    bool m_reactive { false };
    bool m_hasParentSize { false };
    bool m_hasParentConfigureSerial { false };
};

#endif // LPOSITIONER_H
