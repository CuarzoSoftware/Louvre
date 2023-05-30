#ifndef SURFACERESOURCE_H
#define SURFACERESOURCE_H

#include <LResource.h>

class Louvre::Protocols::Wayland::RSurface : public LResource
{
public:
    RSurface(GCompositor *compositorGlobal, UInt32 id);
    ~RSurface();

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

    LPRIVATE_IMP(RSurface)
};

#endif // SURFACERESOURCE_H
