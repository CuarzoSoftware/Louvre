#ifndef RDATASOURCE_H
#define RDATASOURCE_H

#include <LResource.h>
#include <stdio.h>

class Louvre::Protocols::Wayland::RDataSource : public LResource
{
public:

    enum Usage
    {
        Undefined,
        Clipboard,
        DND
    };

    struct MimeTypeFile
    {
        std::string mimeType;
        FILE *tmp = NULL;
    };

    RDataSource(GDataDeviceManager *gDataDeviceManager, UInt32 id);
    ~RDataSource();

    void requestPersistentMimeType(MimeTypeFile &mimeType) noexcept;

    Usage usage() const noexcept;

    // DND only
    UInt32 actions() const noexcept;

    // Since 1
    bool target(const char *mimeType);
    bool send(const char *mimeType, Int32 fd);
    bool cancelled();

    // Since 3
    bool dndDropPerformed();
    bool dndFinished();
    bool action(UInt32 dndAction);

    LPRIVATE_IMP_UNIQUE(RDataSource)
};

#endif // RDATASOURCE_H
