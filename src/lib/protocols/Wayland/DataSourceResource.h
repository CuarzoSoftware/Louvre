#ifndef DATASOURCERESOURCE_H
#define DATASOURCERESOURCE_H

#include <LResource.h>

class Louvre::Protocols::Wayland::DataSourceResource : public LResource
{
public:
    DataSourceResource(DataDeviceManagerGlobal *dataDeviceManagerGlobal, UInt32 id);
    ~DataSourceResource();

    // Events
    void sendDNDDropPerformed();
    void sendCancelled();
    void sendAction(UInt32 action);
    void sendSend(const char *mimeType, Int32 fd);

    LDataSource *dataSource() const;

    LPRIVATE_IMP(DataSourceResource)
};


#endif // DATASOURCERESOURCE_H
