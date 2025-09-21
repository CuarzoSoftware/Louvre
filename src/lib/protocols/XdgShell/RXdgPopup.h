#ifndef RXDGPOPUP_H
#define RXDGPOPUP_H

#include <LResource.h>
#include <LWeak.h>

#include <memory>

class Louvre::Protocols::XdgShell::RXdgPopup final : public LResource {
 public:
  RXdgSurface *xdgSurfaceRes() const noexcept { return m_xdgSurfaceRes; }

  LPopupRole *popupRole() const noexcept { return m_popupRole.get(); }

  /******************** REQUESTS ********************/

  static void destroy(wl_client *client, wl_resource *resource);
  static void grab(wl_client *client, wl_resource *resource, wl_resource *seat,
                   UInt32 serial);

#if LOUVRE_XDG_WM_BASE_VERSION >= 3
  static void reposition(wl_client *client, wl_resource *resource,
                         wl_resource *positioner, UInt32 token);
#endif

  /******************** EVENTS ********************/

  // Since 1
  void configure(const LRect &rect) noexcept;
  void popupDone() noexcept;

  // Since 3
  bool repositioned(UInt32 token) noexcept;

 private:
  friend class Louvre::Protocols::XdgShell::RXdgSurface;
  RXdgPopup(RXdgSurface *xdgSurfaceRes, RXdgSurface *xdgParentSurfaceRes,
            RXdgPositioner *xdgPositionerRes, UInt32 id);
  ~RXdgPopup();

  std::unique_ptr<LPopupRole> m_popupRole;
  LWeak<RXdgSurface> m_xdgSurfaceRes;
};

#endif  // RXDGPOPUP_H
