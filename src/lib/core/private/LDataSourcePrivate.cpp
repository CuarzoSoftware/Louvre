#include <private/LDataSourcePrivate.h>
#include <cstdio>

void LDataSource::LDataSourcePrivate::removeSources()
{
    while(!sources.empty())
    {
        delete []sources.back().mimeType;
        fclose(sources.back().tmp);
        sources.pop_back();
    }
}
