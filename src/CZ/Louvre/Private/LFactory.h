#ifndef LFACTORY_H
#define LFACTORY_H

#include <CZ/Louvre/LSeat.h>
#include <CZ/Louvre/LPointer.h>
#include <CZ/Louvre/LKeyboard.h>
#include <CZ/Louvre/LTouch.h>
#include <CZ/Louvre/LClipboard.h>
#include <CZ/Louvre/LCompositor.h>

namespace Louvre
{
    namespace LFactory
    {
        Int32 objectBeingCreatedType() noexcept;
        void setObjectBeingCreatedType(Int32 type) noexcept;

        template<class T>
        static T *createObject(const void *params)
        {
            // This variable is used to assert that objects are always created by the factory and nowhere else
            setObjectBeingCreatedType((Int32)T::FactoryObjectType);

            // Ask the user to provide its derived type
            T *object { static_cast<T*>(compositor()->createObjectRequest(T::FactoryObjectType, params)) };

            // If nullptr, use the base class
            if (!object)
                object = new T(params);

            setObjectBeingCreatedType(-1);
            return object;
        }
    }
};

#endif // LFACTORY_H
