#ifndef RDATAOFFER_H
#define RDATAOFFER_H

#include <protocols/Wayland/RDataSource.h>
#include <LResource.h>

class Louvre::Protocols::Wayland::RDataOffer : public LResource
{
public:
    RDataOffer(RDataDevice *rDataDevice, UInt32 id, RDataSource::Usage usage);
    ~RDataOffer();

    RDataDevice *dataDeviceResource() const;
    RDataSource::Usage usage() const noexcept;

    // DND only
    UInt32 actions() const noexcept;
    UInt32 preferredAction() const noexcept;
    bool matchedMimeType() const noexcept;

    // Since 1
    bool offer(const char *mimeType);

    // Since 3
    bool sourceActions(UInt32 sourceActions);
    bool action(UInt32 dndAction);

    LPRIVATE_IMP_UNIQUE(RDataOffer)
};

#endif // RDATAOFFER_H
