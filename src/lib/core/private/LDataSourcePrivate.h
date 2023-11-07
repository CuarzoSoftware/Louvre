#ifndef LDATASOURCEPRIVATE_H
#define LDATASOURCEPRIVATE_H

#include <LDataSource.h>

#define LOUVRE_DND_NO_ACTION_SET 4294967295

using namespace Louvre;

LPRIVATE_CLASS(LDataSource)

    void removeSources();
    void removeClientOnlySources();

    Protocols::Wayland::RDataSource *dataSourceResource  = nullptr;
    std::list<LSource>sources;

    // Since 3
    UInt32 dndActions = LOUVRE_DND_NO_ACTION_SET;
};

#endif // LDATASOURCEPRIVATE_H
