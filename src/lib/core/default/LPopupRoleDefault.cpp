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

//! [rolePosC]
const LPoint &LPopupRole::rolePosC() const
{
    // Final position of the popup that we will change if it is restricted
    LPoint finalPos;

    // Position of the parent (without the role option, we will assign it at the end)
    LPoint parentPos = surface()->parent()->posC();

    // Point within the anchor rectangle
    LPoint anchorPos;

    // Popup origin point
    LPoint popupOrigin;

    // Additional offset of the popup given by the positioner
    LPoint offset = positioner().offsetC();

    // Size of the popup (equivalent to its geometry without decorations)
    LSize popupSize = positioner().sizeC();

    // Anchor type flags
    UInt32 anchor = positioner().anchor();

    // Popup gravity flags
    UInt32 gravity = positioner().gravity();

    UInt32 anchorAfterX = positioner().anchor();
    UInt32 gravityAfterX = positioner().gravity();

    // Stores the repositioning attempts of each axis (0, 1, 2, ...) in each phase, another way of repositioning is attempted
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
            anchorPos = positioner().anchorRectC().size()/2;
        }break;
        // Center of the top edge
        case LPositioner::AnchorTop:
        {
            anchorPos.setX(positioner().anchorRectC().w()/2);
        }break;
        // Center of the bottom edge ...
        case LPositioner::AnchorBottom:
        {
            anchorPos.setX(positioner().anchorRectC().w()/2);
            anchorPos.setY(positioner().anchorRectC().h());
        }break;
        case LPositioner::AnchorLeft:
        {
            anchorPos.setY(positioner().anchorRectC().h()/2);
        }break;
        case LPositioner::AnchorRight:
        {
            anchorPos.setX(positioner().anchorRectC().w());
            anchorPos.setY(positioner().anchorRectC().h()/2);
        }break;
        case LPositioner::AnchorTopLeft:
        {
            // (0,0)
        }break;
        case LPositioner::AnchorBottomLeft:
        {
            anchorPos.setY(positioner().anchorRectC().h());
        }break;
        case LPositioner::AnchorTopRight:
        {
            anchorPos.setX(positioner().anchorRectC().w());
        }break;
        case LPositioner::AnchorBottomRight:
        {
            anchorPos = positioner().anchorRectC().size();
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
    finalPos = parentPos + positioner().anchorRectC().pos() + anchorPos - popupOrigin + offset;// - m_imp->windowGeometry.topLeft();

    // If it is the first attempt, we save finalPos in m_rolePosC as a backup
    if (xTry == 0 && yTry == 0)
        m_rolePosC = finalPos;

    /* The positionerBounds rect indicates the space in which the popup should be located and is assigned by the developer (the default library assigns the
     * size of the monitor in which the cursor is located */

    // If the popup exceeds the left border
    if (finalPos.x() < positionerBoundsC().x())
    {

        // First try
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
        else
        {
            gravity = positioner().gravity();
            anchor = positioner().anchor();
            anchorAfterX = anchor;
            gravityAfterX = gravity;
        }

        if (xTry == 1)
        {
            xTry++;

            if (positioner().constraintAdjustment() & LPositioner::ConstraintAdjustment::SlideX)
            {
                offset.setX( offset.x() + (positionerBoundsC().x() - m_rolePosC.x()));
                goto retry;
            }
        }
        else
        {
            offset.setX(positioner().offsetC().x());
        }


    }
    // If right side is constrained
    else if (finalPos.x() + popupSize.w() > positionerBoundsC().x() + positionerBoundsC().w())
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
                offset.setX( offset.x() - ( (m_rolePosC.x()+popupSize.w()) - (positionerBoundsC().x() + positionerBoundsC().w())));
                goto retry;
            }
        }
        else
        {
            offset.setX(positioner().offsetC().x());
        }
    }

    // If top border is constrained
    if (finalPos.y() < positionerBoundsC().y())
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
                offset.setY( offset.y() + (positionerBoundsC().y() - m_rolePosC.y()));
                goto retry;
            }
        }
        else
        {
            offset.setY(positioner().offsetC().y());
        }


    }

    // If bottom border is constrained
    else if (finalPos.y() + popupSize.h() > positionerBoundsC().y() + positionerBoundsC().h())
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
                offset.setY( offset.y() - ( (m_rolePosC.y()+popupSize.h()) - (positionerBoundsC().y() + positionerBoundsC().h())));
                goto retry;
            }
        }
        else
        {
            offset.setY(positioner().offsetC().y());
        }
    }


    m_rolePosC = finalPos;
    surface()->setPosC(m_rolePosC);

    m_rolePosC = finalPos - windowGeometryC().topLeft();

    return m_rolePosC;
}
//! [rolePosC]

//! [pong]
void LPopupRole::pong(UInt32)
{
    /* No default implementation */
}
//! [pong]

//! [grabSeatRequest]
void LPopupRole::grabSeatRequest()
{
    seat()->keyboard()->setFocus(surface());
}
//! [grabSeatRequest]

//! [configureRequest]
void LPopupRole::configureRequest()
{
    setPositionerBoundsC(compositor()->cursor()->output()->rectC());

    LPoint p = rolePosC() - surface()->parent()->posC();

    configureC(LRect(p, positioner().sizeC()));

    compositor()->raiseSurface(surface());
}
//! [configureRequest]

#if LOUVRE_XDG_WM_BASE_VERSION >= 3
//! [repositionRequest]
void LPopupRole::repositionRequest(UInt32 token)
{
    LPoint p = rolePosC() - surface()->parent()->posC();

    sendRepositionedEvent(token);

    configureC(LRect(p,positioner().sizeC()));
}
//! [repositionRequest]
#endif

//! [geometryChanged]
void LPopupRole::geometryChanged()
{
    /* No default implementation */
}
//! [geometryChanged]

