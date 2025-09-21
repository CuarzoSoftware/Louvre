#ifndef GPOINTERCONSTRAINTS_H
#define GPOINTERCONSTRAINTS_H

#include <LResource.h>

class Louvre::Protocols::PointerConstraints::GPointerConstraints final
    : public LResource {
 public:
  static void destroy(wl_client *client, wl_resource *resource) noexcept;
  static void lock_pointer(wl_client *client, wl_resource *resource, UInt32 id,
                           wl_resource *surface, wl_resource *pointer,
                           wl_resource *region, UInt32 lifetime) noexcept;
  static void confine_pointer(wl_client *client, wl_resource *resource,
                              UInt32 id, wl_resource *surface,
                              wl_resource *pointer, wl_resource *region,
                              UInt32 lifetime) noexcept;

 private:
  LGLOBAL_INTERFACE
  GPointerConstraints(wl_client *client, Int32 version, UInt32 id) noexcept;
  ~GPointerConstraints() noexcept;
};

#endif  // GPOINTERCONSTRAINTS_H
