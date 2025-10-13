#include <CZ/Louvre/Protocols/IdleNotify/ext-idle-notify-v1.h>
#include <CZ/Louvre/Protocols/IdleNotify/RIdleNotification.h>
#include <CZ/Louvre/Protocols/IdleNotify/GIdleNotifier.h>
#include <CZ/Louvre/Private/LClientPrivate.h>
#include <CZ/Louvre/LCompositor.h>
#include <CZ/Louvre/LLog.h>
#include <CZ/Core/Utils/CZVectorUtils.h>

using namespace CZ::Protocols::IdleNotify;
using namespace CZ;

static const struct ext_idle_notifier_v1_interface imp
{
    .destroy = &GIdleNotifier::destroy,
    .get_idle_notification = &GIdleNotifier::get_idle_notification
};

LGLOBAL_INTERFACE_IMP(GIdleNotifier, LOUVRE_IDLE_NOTIFIER_VERSION, ext_idle_notifier_v1_interface)

bool GIdleNotifier::Probe(CZWeak<LGlobal> **slot) noexcept
{
    if (compositor()->wellKnownGlobals.IdleNotifier)
    {
        LLog(CZError, CZLN, "Failed to create {} global (already created)", Interface()->name);
        return false;
    }

    *slot = &compositor()->wellKnownGlobals.IdleNotifier;
    return true;
}

GIdleNotifier::GIdleNotifier(
    wl_client *client,
    Int32 version,
    UInt32 id
    )
    :LResource
    (
        client,
        Interface(),
        version,
        id,
        &imp
    )
{
    this->client()->imp()->idleNotifierGlobals.emplace_back(this);
}

GIdleNotifier::~GIdleNotifier() noexcept
{
    CZVectorUtils::RemoveOneUnordered(client()->imp()->idleNotifierGlobals, this);
}

/******************** REQUESTS ********************/

void GIdleNotifier::destroy(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}

void GIdleNotifier::get_idle_notification(wl_client */*client*/, wl_resource *resource, UInt32 id, UInt32 timeout, wl_resource */*seat*/) noexcept
{
    new RIdleNotification(
        static_cast<GIdleNotifier*>(wl_resource_get_user_data(resource))->client(),
        wl_resource_get_version(resource),
        id,
        timeout);
}
