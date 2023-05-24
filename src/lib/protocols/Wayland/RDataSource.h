#ifndef DATASOURCERESOURCE_H
#define DATASOURCERESOURCE_H

#include <LResource.h>

class Louvre::Protocols::Wayland::RDataSource : public LResource
{
public:
    RDataSource(GDataDeviceManager *dataDeviceManagerGlobal, UInt32 id);
    ~RDataSource();

    // Events
    void sendDNDDropPerformed();
    void sendCancelled();
    void sendAction(UInt32 action);
    void sendSend(const char *mimeType, Int32 fd);

    LDataSource *dataSource() const;

    LPRIVATE_IMP(RDataSource)
};


#endif // DATASOURCERESOURCE_H
