#include <private/LDataSourcePrivate.h>
#include <protocols/Wayland/RDataSource.h>

using namespace Louvre;

LDataSource::LDataSource(Wayland::RDataSource *dataSourceResource)
{
    m_imp = new LDataSourcePrivate();
    imp()->dataSourceResource = dataSourceResource;
}

LDataSource::~LDataSource()
{
    imp()->removeSources();
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

// Since 3
UInt32 LDataSource::dndActions() const
{
    return imp()->dndActions;
}

Wayland::RDataSource *LDataSource::dataSourceResource() const
{
    return imp()->dataSourceResource;
}
