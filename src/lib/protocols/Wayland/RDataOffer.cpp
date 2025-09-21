#include <LClient.h>
#include <LDNDSession.h>
#include <fcntl.h>
#include <protocols/Wayland/GSeat.h>
#include <protocols/Wayland/RDataDevice.h>
#include <protocols/Wayland/RDataOffer.h>
#include <sys/poll.h>
#include <sys/sendfile.h>

using namespace Louvre;
using namespace Louvre::Protocols::Wayland;

static const struct wl_data_offer_interface imp{
    .accept = &RDataOffer::accept,
    .receive = &RDataOffer::receive,
    .destroy = &RDataOffer::destroy,
#if LOUVRE_WL_DATA_DEVICE_MANAGER_VERSION >= 3
    .finish = &RDataOffer::finish,
    .set_actions = &RDataOffer::set_actions
#endif
};

RDataOffer::RDataOffer(RDataDevice *dataDeviceRes, UInt32 id,
                       RDataSource::Usage usage) noexcept
    : LResource(dataDeviceRes->client(), &wl_data_offer_interface,
                dataDeviceRes->version(), id, &imp),
      m_dataDeviceRes(dataDeviceRes),
      m_usage(usage) {}

RDataOffer::~RDataOffer() noexcept {
  if (m_dndSession && m_dndSession->dropped && m_dndSession->source)
    m_dndSession->source->cancelled();
}

/******************** REQUESTS ********************/

void RDataOffer::destroy(wl_client * /*client*/,
                         wl_resource *resource) noexcept {
  wl_resource_destroy(resource);
}

void RDataOffer::accept(wl_client * /*client*/, wl_resource *resource,
                        UInt32 /*serial*/, const char *mime_type) noexcept {
  auto &dataOfferRes{
      *static_cast<RDataOffer *>(wl_resource_get_user_data(resource))};
  dataOfferRes.m_matchedMimeType = mime_type != NULL;

  if (dataOfferRes.m_dndSession && dataOfferRes.m_dndSession->source)
    dataOfferRes.m_dndSession->source->target(mime_type);
}

#if LOUVRE_WL_DATA_DEVICE_MANAGER_VERSION >= 3
void RDataOffer::finish(wl_client * /*client*/,
                        wl_resource *resource) noexcept {
  auto &dataOfferRes{
      *static_cast<RDataOffer *>(wl_resource_get_user_data(resource))};

  if (dataOfferRes.usage() != RDataSource::DND) {
    dataOfferRes.postError(WL_DATA_OFFER_ERROR_INVALID_FINISH,
                           "Data offer not used for DND.");
    return;
  }

  if (dataOfferRes.m_dndSession && dataOfferRes.m_dndSession->source)
    dataOfferRes.m_dndSession->source->dndFinished();

  if (dataOfferRes.dataDeviceRes()) dataOfferRes.dataDeviceRes()->leave();

  dataOfferRes.m_dndSession.reset();
}
#endif

void RDataOffer::receive(wl_client * /*client*/, wl_resource *resource,
                         const char *requestedMimeType, Int32 fd) noexcept {
  auto &dataOfferRes{
      *static_cast<RDataOffer *>(wl_resource_get_user_data(resource))};

  if (dataOfferRes.usage() == RDataSource::DND) {
    if (dataOfferRes.m_dndSession && dataOfferRes.m_dndSession->source)
      dataOfferRes.m_dndSession->source->send(requestedMimeType, fd);
  } else if (dataOfferRes.usage() == RDataSource::Clipboard) {
    // Louvre keeps a copy of the source clipboard for each mime type (so we
    // don't ask the source client to write the data)
    for (const auto &mimeType : seat()->clipboard()->mimeTypes()) {
      if (mimeType.mimeType == requestedMimeType) {
        if (seat()->clipboard()->m_dataSource) {
          seat()->clipboard()->m_dataSource->send(requestedMimeType, fd);
        } else if (mimeType.tmp) {
          // Set fd to non-blocking
          int fdFlags{fcntl(fd, F_GETFL, 0)};
          if (fdFlags == -1) goto skip;
          if (fcntl(fd, F_SETFL, fdFlags | O_NONBLOCK) == -1) goto skip;

          const int tmpFd{fileno(mimeType.tmp)};
          fflush(mimeType.tmp);
          fseek(mimeType.tmp, 0L, SEEK_END);
          const long end{ftell(mimeType.tmp)};
          rewind(mimeType.tmp);

          if (end <= 0) break;

          off64_t total = end;
          off64_t offset{0};
          ssize_t sent;

          // Just in case errno = EINTR forever
          UInt32 retryCount{0};
          pollfd pfd = {.fd = fd, .events = POLLOUT};

          while (retryCount < 50 && offset < total) {
            if (poll(&pfd, 1, 500) <= 0) break;

            sent = sendfile(fd, tmpFd, &offset, total - offset);
            if (sent < 0) {
              // Interrupted, try again
              if (errno == EINTR) {
                retryCount++;
                continue;
              }

              break;
            }

            if (sent == 0) break;
          }

          // Restore original blocking mode
          fcntl(fd, F_SETFL, fdFlags);
        }
      skip:
        break;
      }
    }
  }

  close(fd);
}

#if LOUVRE_WL_DATA_DEVICE_MANAGER_VERSION >= 3
void RDataOffer::set_actions(wl_client * /*client*/, wl_resource *resource,
                             UInt32 dnd_actions,
                             UInt32 preferred_action) noexcept {
  auto &dataOfferRes{
      *static_cast<RDataOffer *>(wl_resource_get_user_data(resource))};

  if (dataOfferRes.usage() != RDataSource::DND) {
    dataOfferRes.postError(0, "Data offer not being used for DND.");
    return;
  }

  if (dataOfferRes.actions() == dnd_actions &&
      dataOfferRes.preferredAction() == preferred_action)
    return;

  dnd_actions &= LDND::Copy | LDND::Move | LDND::Ask;

  if (preferred_action != LDND::NoAction && preferred_action != LDND::Copy &&
      preferred_action != LDND::Move && preferred_action != LDND::Ask) {
    dataOfferRes.postError(WL_DATA_OFFER_ERROR_INVALID_ACTION,
                           "Invalid preferred_action.");
    return;
  }

  dataOfferRes.m_actions = dnd_actions;
  dataOfferRes.m_preferredAction = preferred_action;

  if (dataOfferRes.m_dndSession) dataOfferRes.m_dndSession->updateActions();
}
#endif

/******************** EVENTS ********************/

void RDataOffer::offer(const char *mimeType) noexcept {
  wl_data_offer_send_offer(resource(), mimeType);
}

bool RDataOffer::sourceActions(UInt32 sourceActions) noexcept {
#if LOUVRE_WL_DATA_DEVICE_MANAGER_VERSION >= 3
  if (version() >= 3) {
    wl_data_offer_send_source_actions(resource(), sourceActions);
    return true;
  }
#endif
  L_UNUSED(sourceActions);
  return false;
}

bool RDataOffer::action(UInt32 dndAction) noexcept {
#if LOUVRE_WL_DATA_DEVICE_MANAGER_VERSION >= 3
  if (version() >= 3) {
    wl_data_offer_send_action(resource(), dndAction);
    return true;
  }
#endif
  L_UNUSED(dndAction);
  return false;
}
