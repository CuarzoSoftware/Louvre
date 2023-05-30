#ifndef LDATASOURCEPRIVATE_H
#define LDATASOURCEPRIVATE_H

#include <LDataSource.h>

#define DND_NO_ACTION_SET 4294967295

using namespace Louvre;

LPRIVATE_CLASS(LDataSource)

    void remove();

    Wayland::RDataSource *dataSourceResource  = nullptr;
    std::list<LSource>sources;

#if LOUVRE_DATA_DEVICE_MANAGER_VERSION >= 3
    UInt32 dndActions = DND_NO_ACTION_SET;
#endif

};

#endif // LDATASOURCEPRIVATE_H
