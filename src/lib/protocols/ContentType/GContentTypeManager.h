#ifndef GCONTENTTYPEMANAGER_H
#define GCONTENTTYPEMANAGER_H

#include <LResource.h>

class Louvre::Protocols::ContentType::GContentTypeManager final
    : public LResource {
 public:
  static void destroy(wl_client *client, wl_resource *resource);
  static void get_surface_content_type(wl_client *client, wl_resource *resource,
                                       UInt32 id, wl_resource *surface);

 private:
  LGLOBAL_INTERFACE
  GContentTypeManager(wl_client *client, Int32 version, UInt32 id) noexcept;
  ~GContentTypeManager() noexcept;
};

#endif  // GCONTENTTYPEMANAGER_H
