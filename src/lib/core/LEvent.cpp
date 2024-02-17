#include <LEvent.h>
#include <LPointerEnterEvent.h>
#include <LPointerLeaveEvent.h>
#include <LPointerMoveEvent.h>
#include <LPointerButtonEvent.h>
#include <LPointerScrollEvent.h>

using namespace Louvre;

LEvent *LEvent::copy() const
{
    switch (m_type)
    {
    case Type::Pointer:
        switch (m_subtype)
        {
        case Subtype::Enter:
            return (LEvent*)new LPointerEnterEvent((const LPointerEnterEvent&)*this);
        case Subtype::Leave:
            return (LEvent*)new LPointerLeaveEvent((const LPointerLeaveEvent&)*this);
        case Subtype::Move:
            return (LEvent*)new LPointerMoveEvent((const LPointerMoveEvent&)*this);
        case Subtype::Button:
            return (LEvent*)new LPointerButtonEvent((const LPointerButtonEvent&)*this);
        case Subtype::Scroll:
            return (LEvent*)new LPointerScrollEvent((const LPointerScrollEvent&)*this);
        }
    }

    return nullptr;
}
