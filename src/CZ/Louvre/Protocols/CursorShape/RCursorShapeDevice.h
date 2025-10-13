#ifndef RCURSORSHAPEDEVICE_H
#define RCURSORSHAPEDEVICE_H

#include <CZ/Louvre/LResource.h>

class CZ::Protocols::CursorShape::RCursorShapeDevice final : public LResource
{
public:

    enum Type
    {
        Pointer,
        Tablet
    };

    Type type() const noexcept { return m_type; }

    /******************** REQUESTS ********************/

    static void destroy(wl_client *client, wl_resource *resource);
    static void set_shape(wl_client *client, wl_resource *resource, UInt32 serial, UInt32 shape);

private:
    friend class GCursorShapeManager;
    RCursorShapeDevice(GCursorShapeManager *manager, Type type, UInt32 id) noexcept;
    Type m_type;
};

#endif // RCURSORSHAPEDEVICE_H
