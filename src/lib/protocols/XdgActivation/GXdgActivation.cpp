#include <LActivationTokenManager.h>
#include <LUtils.h>
#include <private/LClientPrivate.h>
#include <protocols/Wayland/RSurface.h>
#include <protocols/XdgActivation/GXdgActivation.h>
#include <protocols/XdgActivation/RXdgActivationToken.h>
#include <protocols/XdgActivation/xdg-activation-v1.h>

using namespace Louvre;
using namespace Louvre::Protocols::XdgActivation;

static const struct xdg_activation_v1_interface imp{
    .destroy = &GXdgActivation::destroy,
    .get_activation_token = &GXdgActivation::get_activation_token,
    .activate = &GXdgActivation::activate,
};

void GXdgActivation::bind(wl_client *client, void * /*data*/, UInt32 version,
                          UInt32 id) noexcept {
  new GXdgActivation(client, version, id);
}

Int32 GXdgActivation::maxVersion() noexcept {
  return LOUVRE_XDG_ACTIVATION_VERSION;
}

const wl_interface *GXdgActivation::interface() noexcept {
  return &xdg_activation_v1_interface;
}

GXdgActivation::GXdgActivation(wl_client *client, Int32 version,
                               UInt32 id) noexcept
    : LResource(client, interface(), version, id, &imp) {
  this->client()->imp()->xdgActivationGlobals.emplace_back(this);
}

GXdgActivation::~GXdgActivation() noexcept {
  LVectorRemoveOneUnordered(client()->imp()->xdgActivationGlobals, this);
}

void GXdgActivation::destroy(wl_client * /*client*/,
                             wl_resource *resource) noexcept {
  wl_resource_destroy(resource);
}

void GXdgActivation::get_activation_token(wl_client * /*client*/,
                                          wl_resource *resource,
                                          UInt32 id) noexcept {
  auto *res{static_cast<GXdgActivation *>(wl_resource_get_user_data(resource))};
  new RXdgActivationToken(res, id);
}

void GXdgActivation::activate(wl_client * /*client*/,
                              wl_resource * /*resource*/, const char *token,
                              wl_resource *surface) {
  const std::string tokenString{token};
  auto it{activationTokenManager()->m_tokens.find(tokenString)};

  if (it == activationTokenManager()->m_tokens.end()) return;

  auto *surfaceRes{
      static_cast<Wayland::RSurface *>(wl_resource_get_user_data(surface))};
  activationTokenManager()->m_token.reset(it->second);
  activationTokenManager()->activateSurfaceRequest(surfaceRes->surface());
  activationTokenManager()->m_token.reset();
}
