#ifndef LOUTPUTMODEPRIVATE_H
#define LOUTPUTMODEPRIVATE_H

#include <LOutputMode.h>

class Louvre::LOutputMode::LOutputModePrivate
{

public:
    LOutputModePrivate() = default;
    ~LOutputModePrivate() = default;

    LOutputModePrivate(const LOutputModePrivate&) = delete;
    LOutputModePrivate& operator= (const LOutputModePrivate&) = delete;

    const LOutput *output = nullptr;
    void *graphicBackendData = nullptr;

};

#endif // LOUTPUTMODEPRIVATE_H
