#include <private/LDataSourcePrivate.h>
#include <cstdio>

void LDataSource::LDataSourcePrivate::removeSources()
{
    while(!sources.empty())
    {
        free(sources.back().mimeType);

        if (sources.back().tmp)
            fclose(sources.back().tmp);

        sources.pop_back();
    }
}

void LDataSource::LDataSourcePrivate::removeClientOnlySources()
{
    for (std::list<LSource>::iterator it = sources.begin(); it != sources.end();)
    {
        if ((*it).tmp == NULL)
        {
            free((*it).mimeType);
            it = sources.erase(it);
        }
        else
            ++it;
    }
}
