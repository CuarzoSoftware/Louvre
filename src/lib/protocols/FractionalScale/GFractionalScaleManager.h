#ifndef GFRACTIONALSCALEMANAGER_H
#define GFRACTIONALSCALEMANAGER_H

#include <LResource.h>

class Louvre::Protocols::FractionalScale::GFractionalScaleManager final
    : public LResource {
 public:
  static void destroy(wl_client *client, wl_resource *resource) noexcept;
  static void get_fractional_scale(wl_client *client, wl_resource *resource,
                                   UInt32 id, wl_resource *surface) noexcept;

 private:
  LGLOBAL_INTERFACE
  GFractionalScaleManager(wl_client *client, Int32 version, UInt32 id) noexcept;
  ~GFractionalScaleManager() noexcept;
};

#endif  // GFRACTIONALSCALEMANAGER_H
