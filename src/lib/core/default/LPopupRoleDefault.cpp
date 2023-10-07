#include <protocols/Wayland/GSeat.h>
#include <private/LPopupRolePrivate.h>
#include <private/LSurfacePrivate.h>
#include <private/LBaseSurfaceRolePrivate.h>
#include <LPositioner.h>
#include <LSeat.h>
#include <LKeyboard.h>
#include <LCompositor.h>
#include <LPointer.h>
#include <LCursor.h>
#include <LOutput.h>

using namespace Louvre;

//! [rolePos]
const LPoint &LPopupRole::rolePos() const
{
    if (!surface()->parent())
        return m_rolePos;

    // Final position of the popup that we will change if it is restricted
    LPoint finalPos;

    // Position of the parent (without the role option, we will assign it at the end)
    LPoint parentPos = surface()->parent()->rolePos();

    if (surface()->parent()->toplevel())
        parentPos += surface()->parent()->toplevel()->windowGeometry().topLeft();
    else if (surface()->parent()->popup())
        parentPos += surface()->parent()->popup()->windowGeometry().topLeft();

    // Point within the anchor rectangle
    LPoint anchorPos;

    // Popup origin point
    LPoint popupOrigin;

    // Additional offset of the popup given by the positioner
    LPoint offset = positioner().offset();

    // Size of the popup (equivalent to its geometry without decorations)
    LSize popupSize = positioner().size();

    // Anchor type flags
    UInt32 anchor = positioner().anchor();

    // Popup gravity flags
    UInt32 gravity = positioner().gravity();

    UInt32 anchorAfterX = positioner().anchor();
    UInt32 gravityAfterX = positioner().gravity();

    // Stores the repositioning attempts of each axis (0, 1, 2, ...) in each phase, another way of repositioning is attempted
    // 0 = flip | 1 = slide | 2 = resize
    UInt32 xTry = 0;
    UInt32 yTry = 0;

    // We come back here in case the position is restricted
    retry:
    anchorPos = LPoint();
    popupOrigin = LPoint();

    /* The anchor rect is the space within the parent surface in which the popup can be positioned.
     * The following switch calculates the point within that rect that the popup will take as reference.
     * Depending on the flag, the point can be one of the corners, the middle of some edges, or the center.*/
    switch(anchor)
    {
        // Center of the rect
        case LPositioner::Anchor::NoAnchor:
        {
            anchorPos = positioner().anchorRect().size()/2;
        }break;
        // Center of the top edge
        case LPositioner::AnchorTop:
        {
            anchorPos.setX(positioner().anchorRect().w()/2);
        }break;
        // Center of the bottom edge ...
        case LPositioner::AnchorBottom:
        {
            anchorPos.setX(positioner().anchorRect().w()/2);
            anchorPos.setY(positioner().anchorRect().h());
        }break;
        case LPositioner::AnchorLeft:
        {
            anchorPos.setY(positioner().anchorRect().h()/2);
        }break;
        case LPositioner::AnchorRight:
        {
            anchorPos.setX(positioner().anchorRect().w());
            anchorPos.setY(positioner().anchorRect().h()/2);
        }break;
        case LPositioner::AnchorTopLeft:
        {
            // (0,0)
        }break;
        case LPositioner::AnchorBottomLeft:
        {
            anchorPos.setY(positioner().anchorRect().h());
        }break;
        case LPositioner::AnchorTopRight:
        {
            anchorPos.setX(positioner().anchorRect().w());
        }break;
        case LPositioner::AnchorBottomRight:
        {
            anchorPos = positioner().anchorRect().size();
        }break;
    }

    /* The following switch calculates the gravity of the popup, in other words, in which direction it should try to move.
     * For example, if the point of the anchor rect is the top-left corner and the gravity of the popup is down-right,
     * the popup would be positioned at the coordinate (0,0) of the anchor rect.
     * You can imagine that the point of the anchor rect is a "nail" and that the popup is a frame made up of edges only,
     * depending on the gravity, the popup will move in one direction, but the nail will prevent it from moving further
     * because it will collide with the edges.*/
    switch(gravity)
    {
        case LPositioner::Gravity::NoGravity:
        {
            popupOrigin = popupSize/2;
        }break;
        case LPositioner::GravityTop:
        {
            popupOrigin.setX(popupSize.w()/2);
            popupOrigin.setY(popupSize.h());
        }break;
        case LPositioner::GravityBottom:
        {
            popupOrigin.setX(popupSize.w()/2);
        }break;
        case LPositioner::GravityLeft:
        {
            popupOrigin.setX(popupSize.w());
            popupOrigin.setY(popupSize.h()/2);
        }break;
        case LPositioner::GravityRight:
        {
            popupOrigin.setY(popupSize.h()/2);
        }break;
        case LPositioner::GravityTopLeft:
        {
            popupOrigin = popupSize;
        }break;
        case LPositioner::GravityBottomLeft:
        {
            popupOrigin.setX(popupSize.w());
        }break;
        case LPositioner::GravityTopRight:
        {
            popupOrigin.setY(popupSize.h());
        }break;
        case LPositioner::GravityBottomRight:
        {
            // (0,0)
        }break;
    }

    // We calculate the initial position (we will be modifying finalPos if the position is restricted)
    finalPos = parentPos + positioner().anchorRect().pos() + anchorPos - popupOrigin + offset;

    // If it is the first attempt, we save finalPos in m_rolePosC as a backup
    if (xTry == 0 && yTry == 0)
        m_rolePos = finalPos;

    /* The positionerBounds rect indicates the space in which the popup should be located and is assigned by the developer (the default library assigns the
     * size of the monitor in which the cursor is located */

    // If the popup exceeds the left border
    if (finalPos.x() < positionerBounds().x())
    {
        // First try (flip)
        if (xTry == 0)
        {
            xTry++;

            if (positioner().constraintAdjustment() & LPositioner::ConstraintAdjustment::FlipX)
            {
                switch(anchor)
                {
                    case LPositioner::AnchorLeft:
                    {
                        anchor = LPositioner::AnchorRight;
                    }break;
                    case LPositioner::AnchorTopLeft:
                    {
                        anchor = LPositioner::AnchorTopRight;
                    }break;
                    case LPositioner::AnchorBottomLeft:
                    {
                        anchor = LPositioner::AnchorBottomRight;
                    }break;
                }

                switch(gravity)
                {
                    case LPositioner::GravityLeft:
                    {
                        gravity = LPositioner::GravityRight;
                    }break;
                    case LPositioner::GravityTopLeft:
                    {
                        gravity = LPositioner::GravityTopRight;
                    }break;
                    case LPositioner::GravityBottomLeft:
                    {
                        gravity = LPositioner::GravityBottomRight;
                    }break;
                }

                anchorAfterX = anchor;
                gravityAfterX = gravity;
                goto retry;
            }
        }
        // If not the 0 case, reset vars
        else
        {
            gravity = positioner().gravity();
            anchor = positioner().anchor();
            anchorAfterX = anchor;
            gravityAfterX = gravity;
        }

        // Seccond try (slide)
        if (xTry == 1)
        {
            xTry++;

            if (positioner().constraintAdjustment() & LPositioner::ConstraintAdjustment::SlideX)
            {
                offset.setX( offset.x() + (positionerBounds().x() - m_rolePos.x()));
                goto retry;
            }
        }
        else
        {
            offset.setX(positioner().offset().x());
        }
    }
    // If right side is constrained
    else if (finalPos.x() + popupSize.w() > positionerBounds().x() + positionerBounds().w())
    {
        if (xTry == 0)
        {
            xTry++;

            if (positioner().constraintAdjustment() & LPositioner::ConstraintAdjustment::FlipX)
            {
                switch(anchor)
                {
                    case LPositioner::AnchorRight:
                    {
                        anchor = LPositioner::AnchorLeft;
                    }break;
                    case LPositioner::AnchorTopRight:
                    {
                        anchor = LPositioner::AnchorTopLeft;
                    }break;
                    case LPositioner::AnchorBottomRight:
                    {
                        anchor = LPositioner::AnchorBottomLeft;
                    }break;
                }

                switch(gravity)
                {
                    case LPositioner::GravityRight:
                    {
                        gravity = LPositioner::GravityLeft;
                    }break;
                    case LPositioner::GravityTopRight:
                    {
                        gravity = LPositioner::GravityTopLeft;
                    }break;
                    case LPositioner::GravityBottomRight:
                    {
                        gravity = LPositioner::GravityBottomLeft;
                    }break;
                }

                anchorAfterX = anchor;
                gravityAfterX = gravity;
                goto retry;
            }
        }
        else
        {
            gravity = positioner().gravity();
            anchor = positioner().anchor();
            anchorAfterX = anchor;
            gravityAfterX = gravity;
        }

        // Try slideX
        if (xTry == 1)
        {
            xTry++;

            if (positioner().constraintAdjustment() & LPositioner::ConstraintAdjustment::SlideX)
            {
                offset.setX( offset.x() - ( (m_rolePos.x()+popupSize.w()) - (positionerBounds().x() + positionerBounds().w())));
                goto retry;
            }
        }
        else
        {
            offset.setX(positioner().offset().x());
        }
    }

    // If top border is constrained
    if (finalPos.y() < positionerBounds().y())
    {
        // Flip gravity on the Y axis
        if (yTry == 0)
        {
            yTry++;

            if (positioner().constraintAdjustment() & LPositioner::ConstraintAdjustment::FlipY)
            {
                switch(anchor)
                {
                    case LPositioner::AnchorTop:
                    {
                        anchor = LPositioner::AnchorBottom;
                    }break;
                    case LPositioner::AnchorTopLeft:
                    {
                        anchor = LPositioner::AnchorBottomLeft;
                    }break;
                    case LPositioner::AnchorTopRight:
                    {
                        anchor = LPositioner::AnchorBottomRight;
                    }break;
                }

                switch(gravity)
                {
                    case LPositioner::GravityTop:
                    {
                        gravity = LPositioner::GravityBottom;
                    }break;
                    case LPositioner::GravityTopLeft:
                    {
                        gravity = LPositioner::GravityBottomLeft;
                    }break;
                    case LPositioner::GravityTopRight:
                    {
                        gravity = LPositioner::GravityBottomRight;
                    }break;
                }

                goto retry;
            }
        }
        else
        {
            gravity = gravityAfterX;
            anchor = anchorAfterX;
        }

        if (yTry == 1)
        {
            yTry++;

            if (positioner().constraintAdjustment() & LPositioner::ConstraintAdjustment::SlideY)
            {
                offset.setY( offset.y() + (positionerBounds().y() - m_rolePos.y()));
                goto retry;
            }
        }
        else
        {
            offset.setY(positioner().offset().y());
        }
    }

    // If bottom border is constrained
    else if (finalPos.y() + popupSize.h() > positionerBounds().y() + positionerBounds().h())
    {
        if (yTry == 0)
        {
            yTry++;

            if (positioner().constraintAdjustment() & LPositioner::ConstraintAdjustment::FlipY)
            {
                switch(anchor)
                {
                    case LPositioner::AnchorBottom:
                    {
                        anchor = LPositioner::AnchorTop;
                    }break;
                    case LPositioner::AnchorBottomRight:
                    {
                        anchor = LPositioner::AnchorTopRight;
                    }break;
                    case LPositioner::AnchorBottomLeft:
                    {
                        anchor = LPositioner::AnchorTopLeft;
                    }break;
                }

                switch(gravity)
                {
                    case LPositioner::GravityBottom:
                    {
                        gravity = LPositioner::GravityTop;
                    }break;
                    case LPositioner::GravityBottomRight:
                    {
                        gravity = LPositioner::GravityTopRight;
                    }break;
                    case LPositioner::GravityBottomLeft:
                    {
                        gravity = LPositioner::GravityTopLeft;
                    }break;
                }

                goto retry;
            }
        }
        else
        {
            gravity = gravityAfterX;
            anchor = anchorAfterX;
        }

        if (yTry == 1)
        {
            yTry++;

            if (positioner().constraintAdjustment() & LPositioner::ConstraintAdjustment::SlideY)
            {
                offset.setY( offset.y() - ( (m_rolePos.y()+popupSize.h()) - (positionerBounds().y() + positionerBounds().h())));
                goto retry;
            }
        }
        else
        {
            offset.setY(positioner().offset().y());
        }
    }

    m_rolePos = finalPos;
    surface()->setPos(m_rolePos);
    m_rolePos = finalPos - windowGeometry().topLeft();

    // Finally check if popup size is out of bounds
    LSize finalSize = popupSize;

    if (positioner().constraintAdjustment() & LPositioner::ConstraintAdjustment::ResizeY && finalSize.h() > positionerBounds().h())
    {
        finalSize.setH(positionerBounds().h());
        m_rolePos.setY(positionerBounds().y());
    }

    if (positioner().constraintAdjustment() & LPositioner::ConstraintAdjustment::ResizeX && finalSize.w() > positionerBounds().w())
    {
        finalSize.setW(positionerBounds().w());
        m_rolePos.setX(positionerBounds().x());
    }

    if (finalSize != popupSize)
        configure(LRect(m_rolePos - parentPos, finalSize));

    return m_rolePos;
}
//! [rolePos]

//! [grabSeatRequest]
void LPopupRole::grabSeatRequest(Wayland::GSeat *seatGlobal)
{
    /* The library internally verifies that this request has been
     * originated from some client event, such as a click or key press*/
    seat()->keyboard()->setGrabbingSurface(surface(), seatGlobal->keyboardResource());
}
//! [grabSeatRequest]

//! [configureRequest]
void LPopupRole::configureRequest()
{
    setPositionerBounds(cursor()->output()->rect());
    LPoint p = rolePos() - surface()->parent()->pos();
    configure(LRect(p, positioner().size()));
}
//! [configureRequest]

//! [geometryChanged]
void LPopupRole::geometryChanged()
{
    /* No default implementation */
}
//! [geometryChanged]
