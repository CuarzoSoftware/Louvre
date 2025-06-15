#ifndef LOBJECT_H
#define LOBJECT_H

#include <CZ/CZObject.h>
#include <CZ/Louvre/LNamespaces.h>
#include <vector>

/**
 * @brief Base class for Louvre objects.
 */
class Louvre::LObject : public CZObject
{
public:
    LObject() = default;
};

#endif // LOBJECT_H
