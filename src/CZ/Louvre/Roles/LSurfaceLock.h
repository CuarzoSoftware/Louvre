#ifndef LSURFACELOCK_H
#define LSURFACELOCK_H

#include <CZ/Louvre/LObject.h>
#include <CZ/Core/CZWeak.h>

class CZ::LSurfaceLock : public LObject
{
public:
    UInt32 commitId() const noexcept { return m_commitId; }
    ~LSurfaceLock() noexcept;
private:
    friend class LSurface;
    LSurfaceLock(LSurface *surface, bool userCreated) noexcept;
    CZWeak<LSurface> m_surface;
    UInt32 m_commitId;
    bool m_userCreated;
};

#endif // LSURFACELOCK_H
