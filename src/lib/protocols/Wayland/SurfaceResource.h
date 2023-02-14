#ifndef SURFACERESOURCE_H
#define SURFACERESOURCE_H

#include <LResource.h>

class Louvre::Protocols::Wayland::SurfaceResource : public LResource
{

public:

    SurfaceResource(CompositorGlobal *compositorGlobal, UInt32 id);
    ~SurfaceResource();

    LSurface *surface() const;

    /// @brief Commit origin
    /// Indicates who requests to commit a surface
    enum CommitOrigin
    {
        /// @brief The commit is requested by the surface itself
        Itself,

        /// @brief The commit is requested by the parent surface
        Parent
    };

    LPRIVATE_IMP(SurfaceResource)
};

#endif // SURFACERESOURCE_H
