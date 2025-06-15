#ifndef RCALLBACK_H
#define RCALLBACK_H

#include <LResource.h>

class Louvre::Protocols::Wayland::RCallback final : public LResource
{
public:

    /******************** EVENTS ********************/

    void done(UInt32 data) noexcept;

private:
    friend class Louvre::LSurface;
    friend class Louvre::Protocols::Wayland::RSurface;
    RCallback(wl_client *client, UInt32 id, std::vector<RCallback*> *vector = nullptr) noexcept;
    ~RCallback() noexcept;
    std::vector<RCallback*> *m_vector;
    bool m_commited { false };
};

#endif // RCALLBACK_H
