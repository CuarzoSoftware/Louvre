#ifndef RCALLBACK_H
#define RCALLBACK_H

#include <LResource.h>

class Louvre::Protocols::Wayland::RCallback : public LResource
{
public:
    RCallback(wl_client *client, UInt32 id, std::list<RCallback*>*list);
    ~RCallback();

    bool commited = false;
    bool done(UInt32 data);

    LPRIVATE_IMP(RCallback)
};

#endif // RCALLBACK_H
