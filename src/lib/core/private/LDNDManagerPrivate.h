#ifndef LDNDMANAGERPRIVATE_H
#define LDNDMANAGERPRIVATE_H

#include <LDNDManager.h>
#include <LPointerButtonEvent.h>

using namespace Louvre;

struct LDNDManager::Params
{
    /* Add here any required constructor param */
};

LPRIVATE_CLASS(LDNDManager)

    void clear();

    std::unique_ptr<LEvent> triggeringEvent { std::make_unique<LPointerButtonEvent>() };

    // Since 3
    Action preferredAction                                      { NoAction };

    LDNDIconRole *icon                                          { nullptr };
    LSurface *origin                                            { nullptr };
    LSurface *focus                                             { nullptr };
    LDataSource *source                                         { nullptr };
    LClient *dstClient                                          { nullptr };
    Protocols::Wayland::RDataDevice *srcDataDevice              { nullptr };

    bool dropped                                                { false };
    bool matchedMimeType                                        { false };

    void sendLeaveEvent(LSurface *surface);
};

#endif // LDNDMANAGERPRIVATE_H
