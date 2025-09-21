#ifndef GFOREIGNTOPLEVELMANAGER_H
#define GFOREIGNTOPLEVELMANAGER_H

#include <LResource.h>

class Louvre::Protocols::ForeignToplevelManagement::GForeignToplevelManager
    final : public LResource {
 public:
  /*
   * Checks if stop() has been requested.
   *
   * If true:
   * - Further client requests will be ignored.
   * - New toplevel objects won't be announced.
   * - Changes from already announced toplevels will still be notified.
   */
  bool stopped() const noexcept { return m_stopped; }

  /******************** REQUESTS ********************/

  static void stop(wl_client *client, wl_resource *resource) noexcept;

  /******************** EVENTS ********************/

  /* Creates a new LToplevelController */
  void toplevel(LToplevelRole &toplevelRole);

  /* Notify and destroy the resource */
  const std::vector<GForeignToplevelManager *>::iterator finished();

 private:
  bool m_stopped{false};
  bool m_finished{false};
  LGLOBAL_INTERFACE
  GForeignToplevelManager(wl_client *client, Int32 version, UInt32 id) noexcept;
  ~GForeignToplevelManager() noexcept;
};

#endif  // GFOREIGNTOPLEVELMANAGER_H
