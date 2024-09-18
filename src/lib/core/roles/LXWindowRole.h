#ifndef LXWINDOWROLE_H
#define LXWINDOWROLE_H

#include <LBaseSurfaceRole.h>
#include <memory>

class Louvre::LXWindowRole : public LBaseSurfaceRole
{
public:
    struct Params;
    static constexpr LFactoryObject::Type FactoryObjectType = LFactoryObject::Type::LXWindowRole;

    struct Configuration
    {
        LPoint pos;
        LSize size;
    };

    LXWindowRole(const void *params) noexcept;
    LCLASS_NO_COPY(LXWindowRole)
    ~LXWindowRole();

    virtual const LPoint &rolePos() const override;
    virtual void configureRequest(const Configuration &conf);
    void configure(const LPoint &pos, const LSize &size) noexcept;

    const LPoint &pos() const noexcept;
    const LSize &size() const noexcept;
    UInt32 winId() const noexcept;

    void setSurface(LSurface *surface);

    LPRIVATE_IMP_UNIQUE(LXWindowRole)
    void handleSurfaceCommit(LBaseSurfaceRole::CommitOrigin origin) override;
};

#endif // LXWINDOWROLE_H
