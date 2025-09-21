#ifndef RDATADEVICE_H
#define RDATADEVICE_H

#include <protocols/Wayland/RDataSource.h>

class Louvre::Protocols::Wayland::RDataDevice final : public LResource {
 public:
  GSeat *seatRes() const noexcept { return m_seatRes; }

  UInt32 enterSerial() const noexcept { return m_enterSerial; }

  RDataOffer *createOffer(RDataSource::Usage usage) noexcept;

  /******************** REQUESTS ********************/

  static void start_drag(wl_client *client, wl_resource *resource,
                         wl_resource *source, wl_resource *origin,
                         wl_resource *icon, UInt32 serial);
  static void set_selection(wl_client *client, wl_resource *resource,
                            wl_resource *source, UInt32 serial);

#if LOUVRE_WL_DATA_DEVICE_MANAGER_VERSION >= 2
  static void release(wl_client *client, wl_resource *resource) noexcept;
#endif

  /******************** EVENTS ********************/

  // Since 1
  void dataOffer(RDataOffer *offer) noexcept;
  void enter(UInt32 serial, RSurface *surface, Float24 x, Float24 y,
             RDataOffer *offer) noexcept;
  void leave() noexcept;
  void motion(UInt32 time, Float24 x, Float24 y) noexcept;
  void drop() noexcept;
  void selection(RDataOffer *offer) noexcept;

 private:
  friend class Louvre::Protocols::Wayland::GDataDeviceManager;
  friend class Louvre::LDND;

  RDataDevice(GDataDeviceManager *dataDeviceManagerRes, GSeat *seatRes,
              Int32 id) noexcept;
  ~RDataDevice() noexcept = default;

  LWeak<GSeat> m_seatRes;
  UInt32 m_enterSerial;
};

#endif  // RDATADEVICE_H
