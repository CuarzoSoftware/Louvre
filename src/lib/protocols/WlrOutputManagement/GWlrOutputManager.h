#ifndef GWLROUTPUTMANAGER_H
#define GWLROUTPUTMANAGER_H

#include <LResource.h>
#include <LWeak.h>

class Louvre::Protocols::WlrOutputManagement::GWlrOutputManager final : public LResource
{
public:

    UInt32 lastDoneSerial() const noexcept
    {
        return m_serial;
    }

    /******************** REQUESTS ********************/

    static void create_configuration(wl_client *client, wl_resource *resource, UInt32 id, UInt32 serial);
    static void stop(wl_client *client, wl_resource *resource);

    /******************** EVENTS ********************/

    RWlrOutputHead *head(LOutput *output) noexcept;
    void done() noexcept;
    void finished() noexcept;
private:
    friend class RWlrOutputHead;
    LGLOBAL_INTERFACE
    GWlrOutputManager(wl_client *client, Int32 version, UInt32 id) noexcept;
    ~GWlrOutputManager() noexcept;
    std::vector<RWlrOutputHead*> m_heads;
    UInt32 m_serial;
    bool m_stopped { false };
    bool m_pendingDone { true };
};

#endif // GWLROUTPUTMANAGER_H
