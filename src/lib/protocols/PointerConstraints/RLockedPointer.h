#ifndef RLOCKEDPOINTER_H
#define RLOCKEDPOINTER_H

#include <LResource.h>
#include <LWeak.h>

class Louvre::Protocols::PointerConstraints::RLockedPointer final
    : public LResource {
 public:
  Wayland::RPointer *pointerRes() const noexcept { return m_pointerRes; }

  UInt32 lifetime() const noexcept { return m_lifetime; }

  LSurface *surface() const noexcept { return m_surface; }

  bool constrained() const noexcept { return m_constrained; }

  /******************** REQUESTS ********************/

  static void destroy(wl_client *client, wl_resource *resource);
  static void set_cursor_position_hint(wl_client *client, wl_resource *resource,
                                       Float24 x, Float24 y) noexcept;
  static void set_region(wl_client *client, wl_resource *resource,
                         wl_resource *region) noexcept;

  /******************** EVENTS ********************/

  // Since 1
  void locked() noexcept;
  void unlocked();

 private:
  friend class Louvre::Protocols::PointerConstraints::GPointerConstraints;
  RLockedPointer(GPointerConstraints *pointerConstraintsRes, LSurface *surface,
                 Wayland::RPointer *pointerRes, Wayland::RRegion *regionRes,
                 UInt32 lifetime, UInt32 id) noexcept;
  ~RLockedPointer();
  LWeak<Wayland::RPointer> m_pointerRes;
  LWeak<LSurface> m_surface;
  UInt32 m_lifetime;
  bool m_constrained{false};
};

#endif  // RLOCKEDPOINTER_H
