#ifndef LFOREIGNTOPLEVELCONTROLLER_H
#define LFOREIGNTOPLEVELCONTROLLER_H

#include <LFactoryObject.h>
#include <LRect.h>
#include <LWeak.h>

class Louvre::LForeignToplevelController : public LFactoryObject
{
public:
    static constexpr LFactoryObject::Type FactoryObjectType = LFactoryObject::Type::LForeignToplevelController;

    LForeignToplevelController(const void *params) noexcept;
    ~LForeignToplevelController() noexcept;

    LToplevelRole *toplevelRole() const noexcept;

    LClient *client() const noexcept;

    Protocols::ForeignToplevelManagement::RForeignToplevelHandle &resource() const noexcept
    {
        return m_resource;
    }

    LSurface *taskbar() const noexcept
    {
        return m_taskbar;
    }

    const LRect &taskbarIconRect() const noexcept
    {
        return m_taskbarIconRect;
    }

    virtual void taskbarChanged() {};

private:
    friend class Protocols::ForeignToplevelManagement::RForeignToplevelHandle;
    Protocols::ForeignToplevelManagement::RForeignToplevelHandle &m_resource;
    LRect m_taskbarIconRect;
    LWeak<LSurface> m_taskbar;
};

#endif // LFOREIGNTOPLEVELCONTROLLER_H
