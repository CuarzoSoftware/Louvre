#include <LForeignToplevelController.h>
#include <LToplevelRole.h>
#include <LUtils.h>
#include <protocols/ForeignToplevelManagement/RForeignToplevelHandle.h>

using namespace Louvre;
using namespace Louvre::Protocols::ForeignToplevelManagement;

LForeignToplevelController::LForeignToplevelController(
    const void *params) noexcept
    : LFactoryObject(FactoryObjectType),
      m_resource(*(RForeignToplevelHandle *)(params)) {
  m_taskbar.setOnDestroyCallback([this](auto) { taskbarChanged(); });

  toplevelRole()->m_foreignControllers.emplace_back(this);
}

LForeignToplevelController::~LForeignToplevelController() noexcept {
  notifyDestruction();

  if (toplevelRole())
    LVectorRemoveOneUnordered(toplevelRole()->m_foreignControllers, this);
}

LToplevelRole *LForeignToplevelController::toplevelRole() const noexcept {
  return resource().toplevelRole();
}

LClient *LForeignToplevelController::client() const noexcept {
  return resource().client();
}
