#include <LFramebufferWrapper.h>

#if LOUVRE_USE_SKIA == 1
#include <LPainter.h>
#include <private/LCompositorPrivate.h>
#include <include/gpu/GrBackendSurface.h>
#include <include/gpu/ganesh/SkSurfaceGanesh.h>
#include <include/gpu/gl/GrGLTypes.h>
#include <include/core/SkColorSpace.h>

using namespace Louvre;

sk_sp<SkSurface> LFramebufferWrapper::skSurface() const noexcept
{
    if (!m_skSurface || m_skSurfaceNeedsUpdate)
    {
        LPainter *p { compositor()->imp()->findPainter() };

        if (!p || !p->skContext())
            return nullptr;

        m_skSurfaceNeedsUpdate = false;

        const GrGLFramebufferInfo fbInfo{
            .fFBOID = m_fbId,
            .fFormat = GL_RGBA8_OES
        };

        const GrBackendRenderTarget target(
            m_sizeB.w(),
            m_sizeB.h(),
            0, 0,
            fbInfo);

        static sk_sp<SkColorSpace> skColorSpace = SkColorSpace::MakeSRGB();
        static SkSurfaceProps skSurfaceProps(0, kUnknown_SkPixelGeometry);

        m_skSurface = SkSurfaces::WrapBackendRenderTarget(
            p->skContext(),
            target,
            m_fbId == 0 ? GrSurfaceOrigin::kBottomLeft_GrSurfaceOrigin : GrSurfaceOrigin::kTopLeft_GrSurfaceOrigin,
            SkColorType::kRGBA_8888_SkColorType,
            skColorSpace,
            &skSurfaceProps);
    }

    return m_skSurface;
}
#endif
