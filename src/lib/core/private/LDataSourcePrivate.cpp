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
