#ifndef RFOREIGNTOPLEVELLISTHANDLE_H
#define RFOREIGNTOPLEVELLISTHANDLE_H

#include <LResource.h>
#include <LWeak.h>

#include <string>

class Louvre::Protocols::ForeignToplevelList::RForeignToplevelHandle final
    : public LResource {
 public:
  LToplevelRole *toplevelRole() const noexcept { return m_toplevelRole; }
  bool canSendParams() const noexcept;

  /******************** REQUESTS ********************/

  static void destroy(wl_client *client, wl_resource *resource);

  /******************** EVENTS ********************/

  void closed() noexcept;
  void done() noexcept;
  void title(const std::string &title) noexcept;
  void appId(const std::string &appId) noexcept;
  void identifier(const std::string &identifier) noexcept;

 private:
  friend class GForeignToplevelList;
  RForeignToplevelHandle(GForeignToplevelList &foreignToplevelListRes,
                         LToplevelRole &toplevelRole);
  ~RForeignToplevelHandle();
  LWeak<GForeignToplevelList> m_foreignToplevelListRes;
  LWeak<LToplevelRole> m_toplevelRole;
  bool m_closed{false};
};

#endif  // RFOREIGNTOPLEVELLISTHANDLE_H
