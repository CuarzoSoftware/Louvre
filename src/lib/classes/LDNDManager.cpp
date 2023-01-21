#include <private/LDNDManagerPrivate.h>
#include <private/LDataOfferPrivate.h>
#include <private/LDataDevicePrivate.h>

#include <LSurface.h>
#include <LClient.h>
#include <LDataSource.h>
#include <LSeat.h>
#include <LPointer.h>

using namespace Louvre;

LDNDManager::LDNDManager(Params *params)
{
    m_imp = new LDNDManagerPrivate();
    m_imp->seat = params->seat;
}

LDNDManager::~LDNDManager()
{
    delete m_imp;
}

LDNDIconRole *LDNDManager::icon() const
{
    return m_imp->icon;
}

LSurface *LDNDManager::origin() const
{
    return m_imp->origin;
}

LSurface *LDNDManager::focus() const
{
    return m_imp->focus;
}

LDataSource *LDNDManager::source() const
{
    return m_imp->source;
}

LSeat *LDNDManager::seat() const
{
    return m_imp->seat;
}

bool LDNDManager::dragging() const
{
    return m_imp->origin != nullptr;
}

#if LOUVRE_DATA_DEVICE_MANAGER_VERSION >= 3

Louvre::LDNDManager::Action LDNDManager::preferredAction() const
{
    return m_imp->preferredAction;
}

#endif

void LDNDManager::cancel()
{
    if(m_imp->focus && m_imp->focus->client()->dataDevice())
        m_imp->focus->client()->dataDevice()->imp()->sendDNDLeaveEvent();

    if(source())
        wl_data_source_send_cancelled(source()->resource());

    m_imp->clear();
    seat()->pointer()->setFocusC(nullptr);
    cancelled();
}

void LDNDManager::drop()
{
    if(dragging() && !m_imp->dropped)
    {
        m_imp->dropped = true;

        if(m_imp->focus && m_imp->focus->client()->dataDevice())
        {
            wl_data_device_send_drop(m_imp->focus->client()->dataDevice()->resource());

            #if LOUVRE_DATA_DEVICE_MANAGER_VERSION >= 3
            if(source() && wl_resource_get_version(source()->resource()) >= 3)
            {
                if(m_imp->matchedMimeType)
                    wl_data_source_send_dnd_drop_performed(source()->resource());
                else
                    cancel();
            }
            #endif
        }
        else
        {
            cancel();
        }
    }
}

#if LOUVRE_DATA_DEVICE_MANAGER_VERSION >= 3
void LDNDManager::setPreferredAction(Louvre::LDNDManager::Action action)
{
    m_imp->preferredAction = action;
    if(m_imp->offer)
        m_imp->offer->imp()->updateDNDAction();

}
#endif

void LDNDManager::LDNDManagerPrivate::clear()
{
    focus = nullptr;
    source = nullptr;
    origin = nullptr;
    icon = nullptr;
    offer = nullptr;
    dropped = false;
    matchedMimeType = false;
    destDidNotRequestReceive = 0;
}

LDNDManager::LDNDManagerPrivate *LDNDManager::imp() const
{
    return m_imp;
}
