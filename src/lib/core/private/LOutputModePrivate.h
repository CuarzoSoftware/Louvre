#ifndef LOUTPUTMODEPRIVATE_H
#define LOUTPUTMODEPRIVATE_H

#include <LOutputMode.h>

using namespace Louvre;

LPRIVATE_CLASS(LOutputMode)
    LOutput *output = nullptr;
    void *graphicBackendData = nullptr;
};

#endif // LOUTPUTMODEPRIVATE_H
