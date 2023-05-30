#ifndef LDNDMANAGERPRIVATE_H
#define LDNDMANAGERPRIVATE_H

#include <LDNDManager.h>

using namespace Louvre;

struct LDNDManager::Params
{
    LSeat *seat;
};

LPRIVATE_CLASS(LDNDManager)

    void clear();

    Action preferredAction                                      = NoAction;
    LDNDIconRole *icon                                          = nullptr;
    LSurface *origin                                            = nullptr;
    LSurface *focus                                             = nullptr;
    LDataSource *source                                         = nullptr;
    LClient *dstClient                                          = nullptr;

    bool dropped                                                = false;
    bool matchedMimeType                                        = false;
    UInt32 destDidNotRequestReceive                             = 0;
};

#endif // LDNDMANAGERPRIVATE_H
