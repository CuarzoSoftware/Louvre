#ifndef RSURFACE_H
#define RSURFACE_H

#include <LResource.h>

class Louvre::Protocols::Wayland::RSurface : public LResource
{
public:
    RSurface(GCompositor *gCompositor, UInt32 id);
    ~RSurface();

    LSurface *surface() const;
    Viewporter::RViewport *viewport() const;

    /// @brief Commit origin
    /// Indicates who requests to commit a surface
    enum CommitOrigin
    {
        /// @brief The commit is requested by the surface itself
        Itself,

        /// @brief The commit is requested by the parent surface
        Parent
    };

    // Since 1
    bool enter(GOutput *gOutput);
    bool leave(GOutput *gOutput);

    // Since 6
    bool preferredBufferScale(Int32 scale);
    bool preferredBufferTransform(UInt32 transform);

    LPRIVATE_IMP_UNIQUE(RSurface)
};

#endif // RSURFACE_H
