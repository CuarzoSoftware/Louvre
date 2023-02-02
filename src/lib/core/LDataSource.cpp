#include <private/LDataSourcePrivate.h>
#include <protocols/Wayland/DataSourceResource.h>

using namespace Louvre;

LDataSource::LDataSource(Protocols::Wayland::DataSourceResource *dataSourceResource)
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

Protocols::Wayland::DataSourceResource *LDataSource::dataSourceResource() const
{
    return imp()->dataSourceResource;
}

LDataSource::LDataSourcePrivate *LDataSource::imp() const
{
    return m_imp;
}

/* PRIVATE */

void Louvre::LDataSource::LDataSourcePrivate::remove()
{
    while(!sources.empty())
    {
        delete []sources.back().mimeType;
        fclose(sources.back().tmp);
        sources.pop_back();
    }
}
