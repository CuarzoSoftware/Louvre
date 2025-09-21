#ifndef GFOREIGNTOPLEVELIMAGECAPTURESOURCEMANAGER_H
#define GFOREIGNTOPLEVELIMAGECAPTURESOURCEMANAGER_H

#include <LResource.h>

class Louvre::Protocols::ImageCaptureSource::
    GForeignToplevelImageCaptureSourceManager final : public LResource {
 public:
  static void destroy(wl_client *client, wl_resource *resource) noexcept;
  static void create_source(wl_client *client, wl_resource *resource, UInt32 id,
                            wl_resource *toplevel_handle) noexcept;

 private:
  LGLOBAL_INTERFACE
  GForeignToplevelImageCaptureSourceManager(wl_client *client, Int32 version,
                                            UInt32 id) noexcept;
  ~GForeignToplevelImageCaptureSourceManager() noexcept;
};

#endif  // GFOREIGNTOPLEVELIMAGECAPTURESOURCEMANAGER_H
