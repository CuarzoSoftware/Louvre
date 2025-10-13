#include <CZ/Louvre/Protocols/ForeignToplevelManagement/RForeignToplevelHandle.h>
#include <CZ/Louvre/Roles/LForeignToplevelController.h>
#include <CZ/Louvre/Roles/LToplevelRole.h>
#include <CZ/Core/Utils/CZVectorUtils.h>

using namespace CZ;
using namespace CZ::Protocols::ForeignToplevelManagement;

LForeignToplevelController::LForeignToplevelController(const void *params) noexcept : LFactoryObject(FactoryObjectType),
    m_resource(*(RForeignToplevelHandle*)(params))
{
    m_taskbar.setOnDestroyCallback([this](auto) {
        taskbarChanged();
    });

    toplevelRole()->m_foreignControllers.emplace_back(this);
}

LForeignToplevelController::~LForeignToplevelController() noexcept
{
    notifyDestruction();

    if (toplevelRole())
        CZVectorUtils::RemoveOneUnordered(toplevelRole()->m_foreignControllers, this);
}

LToplevelRole *LForeignToplevelController::toplevelRole() const noexcept
{
    return resource().toplevelRole();
}

LClient *LForeignToplevelController::client() const noexcept
{
    return resource().client();
}
