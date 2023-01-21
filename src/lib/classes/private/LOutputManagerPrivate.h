#ifndef LOUTPUTMANAGERPRIVATE_H
#define LOUTPUTMANAGERPRIVATE_H

#include <LOutputManager.h>

struct Louvre::LOutputManager::Params
{
    LCompositor *compositor;
};

class Louvre::LOutputManager::LOutputManagerPrivate
{
public:
    LOutputManagerPrivate()                                         = default;
    ~LOutputManagerPrivate()                                        = default;

    LOutputManagerPrivate(const LOutputManagerPrivate&)             = delete;
    LOutputManagerPrivate &operator=(const LOutputManagerPrivate&)  = delete;

    LCompositor *compositor                                         = nullptr;
};

#endif // LOUTPUTMANAGERPRIVATE_H
