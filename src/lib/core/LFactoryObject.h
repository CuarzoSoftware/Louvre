#ifndef LFACTORYOBJECT_H
#define LFACTORYOBJECT_H

#include <LObject.h>

class Louvre::LFactoryObject : public LObject
{
public:
    enum class Type : Int32
    {
        LSurface,
        LToplevelRole,
        LPopupRole,
        LSubsurfaceRole,
        LCursorRole,
        LDNDIconRole,
        LSessionLockRole,
        LClient,
        LOutput,
        LSeat,
        LPointer,
        LKeyboard,
        LTouch,
        LClipboard,
        LDND,
        LSessionLockManager
    };

    Type factoryObjectType() const noexcept
    {
        return m_type;
    }

protected:
    LFactoryObject(Type type) noexcept;

private:
    Type m_type;
};

#endif // LFACTORYOBJECT_H
