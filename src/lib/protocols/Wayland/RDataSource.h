#ifndef RDATASOURCE_H
#define RDATASOURCE_H

#include <LResource.h>

class Louvre::Protocols::Wayland::RDataSource : public LResource
{
public:
    RDataSource(GDataDeviceManager *gDataDeviceManager, UInt32 id);
    ~RDataSource();

    LDataSource *dataSource() const;

    // Since 1
    bool target(const char *mimeType);
    bool send(const char *mimeType, Int32 fd);
    bool cancelled();

    // Since 3
    bool dndDropPerformed();
    bool dndFinished();
    bool action(UInt32 dndAction);

    LPRIVATE_IMP(RDataSource)
};

#endif // RDATASOURCE_H
