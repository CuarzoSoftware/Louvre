#ifndef GINVISIBLEREGIONMANAGER_H
#define GINVISIBLEREGIONMANAGER_H

#include <LResource.h>

class Louvre::Protocols::InvisibleRegion::GInvisibleRegionManager final
    : public LResource {
 public:
  static void destroy(wl_client *client, wl_resource *resource);
  static void get_invisible_region(wl_client *client, wl_resource *resource,
                                   UInt32 id, wl_resource *surface);

 private:
  LGLOBAL_INTERFACE
  GInvisibleRegionManager(wl_client *client, Int32 version, UInt32 id) noexcept;
  ~GInvisibleRegionManager() noexcept;
};

#endif  // GINVISIBLEREGIONMANAGER_H
