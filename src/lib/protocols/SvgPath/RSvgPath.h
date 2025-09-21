#ifndef RSVGPATH_H
#define RSVGPATH_H

#include <LBackgroundBlur.h>
#include <LResource.h>

class Louvre::Protocols::SvgPath::RSvgPath final : public LResource {
 public:
  const std::string &commands() const noexcept { return m_commands; }
  bool isComplete() const noexcept { return m_isComplete; }

  /******************** REQUESTS ********************/

  static void destroy(wl_client *client, wl_resource *resource);
  static void concat_commands(wl_client *client, wl_resource *resource,
                              const char *commands);
  static void done(wl_client *client, wl_resource *resource);

 private:
  friend class GSvgPathManager;
  RSvgPath(GSvgPathManager *manager, UInt32 id, Int32 version) noexcept;
  ~RSvgPath() noexcept = default;
  std::string m_commands;
  bool m_isComplete{false};
};

#endif  // RSVGPATH_H
