#include <private/LDataSourcePrivate.h>
#include <protocols/Wayland/RDataSource.h>
#include <cstdio>

using namespace Louvre;

LDataSource::LDataSource(Wayland::RDataSource *dataSourceResource)
{
    m_imp = new LDataSourcePrivate();
    imp()->dataSourceResource = dataSourceResource;
}

LDataSource::~LDataSource()
{
    delete m_imp;
}

LClient *LDataSource::client() const
{
    return dataSourceResource()->client();
}

const std::list<LDataSource::LSource> &LDataSource::sources() const
{
    return imp()->sources;
}

#if LOUVRE_DATA_DEVICE_MANAGER_VERSION >= 3

    UInt32 LDataSource::dndActions() const
    {
        return imp()->dndActions;
    }
#endif

Wayland::RDataSource *LDataSource::dataSourceResource() const
{
    return imp()->dataSourceResource;
}

/* PRIVATE */

void LDataSource::LDataSourcePrivate::remove()
{
    while(!sources.empty())
    {
        delete []sources.back().mimeType;
        fclose(sources.back().tmp);
        sources.pop_back();
    }
}
