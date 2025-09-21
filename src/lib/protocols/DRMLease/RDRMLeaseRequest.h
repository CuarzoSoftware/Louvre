#ifndef RDRMLEASEREQUEST_H
#define RDRMLEASEREQUEST_H

#include <LGPU.h>
#include <LOutput.h>
#include <LResource.h>
#include <LWeak.h>

class Louvre::Protocols::DRMLease::RDRMLeaseRequest final : public LResource {
 public:
  LGPU *gpu() const noexcept { return m_gpu; }

  const std::vector<LWeak<LOutput>> &requestedConnectors() const noexcept {
    return m_requestedConnectors;
  }

  /******************** REQUESTS ********************/

  static void request_connector(wl_client *client, wl_resource *resource,
                                wl_resource *connector);
  static void submit(wl_client *client, wl_resource *resource, UInt32 id);

 private:
  friend class GDRMLeaseDevice;
  RDRMLeaseRequest(GDRMLeaseDevice *leaseDeviceRes, UInt32 id);
  ~RDRMLeaseRequest() = default;
  LWeak<LGPU> m_gpu;
  std::vector<LWeak<LOutput>> m_requestedConnectors;
  bool m_addedConnector{false};
};

#endif  // RDRMLEASEREQUEST_H
