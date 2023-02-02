#ifndef LDATASOURCEPRIVATE_H
#define LDATASOURCEPRIVATE_H

#include <LDataSource.h>
#include <cstdio>

#define DND_NO_ACTION_SET 4294967295

class Louvre::LDataSource::LDataSourcePrivate
{
public:
    LDataSourcePrivate()                                        = default;
    ~LDataSourcePrivate()                                       = default;

    LDataSourcePrivate(const LDataSourcePrivate&)               = delete;
    LDataSourcePrivate &operator=(const LDataSourcePrivate&)    = delete;

    void remove();

    Protocols::Wayland::DataSourceResource *dataSourceResource  = nullptr;
    std::list<LSource>sources;

#if LOUVRE_DATA_DEVICE_MANAGER_VERSION >= 3
    UInt32 dndActions                                           = DND_NO_ACTION_SET;
#endif

};


#endif // LDATASOURCEPRIVATE_H
