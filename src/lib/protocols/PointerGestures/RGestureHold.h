#ifndef RGESTUREHOLD_H
#define RGESTUREHOLD_H

#include <LResource.h>
#include <LWeak.h>

class Louvre::Protocols::PointerGestures::RGestureHold final
    : public LResource {
 public:
  Wayland::RPointer *pointerRes() const noexcept { return m_pointerRes; }

  /******************** REQUESTS ********************/

  static void destroy(wl_client *client, wl_resource *resource) noexcept;

  /******************** EVENTS ********************/

  // Since 1
  void begin(const LPointerHoldBeginEvent &event,
             Wayland::RSurface *surfaceRes) noexcept;
  void end(const LPointerHoldEndEvent &event) noexcept;

 private:
  friend class Louvre::Protocols::PointerGestures::GPointerGestures;
  RGestureHold(Wayland::RPointer *pointerRes, Int32 id,
               UInt32 version) noexcept;
  ~RGestureHold() noexcept;
  LWeak<Wayland::RPointer> m_pointerRes;
};

#endif  // RGESTUREHOLD_H
