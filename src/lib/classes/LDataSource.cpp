#include <private/LDataSourcePrivate.h>

using namespace Louvre;

LDataSource::LDataSource(wl_resource *resource, LClient *client)
{
    m_imp = new LDataSourcePrivate();
    m_imp->resource = resource;
    m_imp->client = client;
}

LDataSource::~LDataSource()
{
    delete m_imp;
}

wl_resource *LDataSource::resource() const
{
    return m_imp->resource;
}

LClient *LDataSource::client() const
{
    return m_imp->client;
}

const std::list<LDataSource::LSource> &LDataSource::sources() const
{
    return m_imp->sources;
}

#if LOUVRE_DATA_DEVICE_MANAGER_VERSION >= 3

    UInt32 LDataSource::dndActions() const
    {
        return m_imp->dndActions;
    }

#endif

LDataSource::LDataSourcePrivate *LDataSource::imp()
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
