#ifndef LOBJECT_H
#define LOBJECT_H

#include <CZ/Core/CZObject.h>
#include <CZ/Louvre/Louvre.h>

/**
 * @brief Base class for Louvre objects.
 */
class CZ::LObject : public CZObject
{
public:
    LObject() = default;
};

#endif // LOBJECT_H
