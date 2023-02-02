#ifndef LDNDMANAGERPRIVATE_H
#define LDNDMANAGERPRIVATE_H

#include <LDNDManager.h>

struct Louvre::LDNDManager::Params
{
    LSeat *seat;
};

class Louvre::LDNDManager::LDNDManagerPrivate
{
public:
    LDNDManagerPrivate()                                        = default;
    ~LDNDManagerPrivate()                                       = default;

    LDNDManagerPrivate(const LDNDManagerPrivate&)               = delete;
    LDNDManagerPrivate &operator=(const LDNDManagerPrivate&)    = delete;

    void clear();

    Action preferredAction                                      = NoAction;
    LDNDIconRole *icon                                          = nullptr;
    LSurface *origin                                            = nullptr;
    LSurface *focus                                             = nullptr;
    LDataSource *source                                         = nullptr;
    LSeat *seat                                                 = nullptr;
    LClient *dstClient                                          = nullptr;

    bool dropped                                                = false;
    bool matchedMimeType                                        = false;
    UInt32 destDidNotRequestReceive                             = 0;

};

#endif // LDNDMANAGERPRIVATE_H
