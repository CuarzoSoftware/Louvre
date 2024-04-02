#ifndef RXDGOUTPUT_H
#define RXDGOUTPUT_H

#include <LResource.h>
#include <LWeak.h>

class Louvre::Protocols::XdgOutput::RXdgOutput final : public LResource
{
public:
    Wayland::GOutput *outputRes() const noexcept { return m_outputRes.get(); }

    /******************** REQUESTS ********************/

    static void destroy(wl_client *client, wl_resource *resource) noexcept;

    /******************** EVENTS ********************/

    // Since 1
    void logicalPosition(const LPoint &pos) noexcept;
    void logicalSize(const LSize &size) noexcept;
    void done() noexcept;

    // Since 2
    bool name(const char *name) noexcept;
    bool description(const char *description) noexcept;

private:
    friend class XdgOutput::GXdgOutputManager;
    friend class Wayland::GOutput;
    RXdgOutput(Wayland::GOutput *outputRes, UInt32 id, Int32 version) noexcept;
    ~RXdgOutput() noexcept;
    LWeak<Wayland::GOutput> m_outputRes;
};

#endif // RXDGOUTPUT_H
