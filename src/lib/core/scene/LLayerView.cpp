#include <LLayerView.h>
#include <LUtils.h>

using namespace Louvre;

bool LLayerView::nativeMapped() const noexcept
{
    return true;
}

const LPoint &LLayerView::nativePos() const noexcept
{
    return m_nativePos;
}

const LSize &LLayerView::nativeSize() const noexcept
{
    return m_nativeSize;
}

Float32 LLayerView::bufferScale() const noexcept
{
    return 1.f;
}

void LLayerView::enteredOutput(LOutput *output) noexcept
{
    LVectorPushBackIfNonexistent(m_outputs, output);
}

void LLayerView::leftOutput(LOutput *output) noexcept
{
    LVectorRemoveOneUnordered(m_outputs, output);
}

const std::vector<LOutput*> &LLayerView::outputs() const noexcept
{
    return m_outputs;
}

void LLayerView::requestNextFrame(LOutput *output) noexcept
{
    L_UNUSED(output);
}

const LRegion *LLayerView::damage() const noexcept
{
    return &LRegion::EmptyRegion();
}

const LRegion *LLayerView::translucentRegion() const noexcept
{
    return &LRegion::EmptyRegion();
}

const LRegion *LLayerView::opaqueRegion() const noexcept
{
    return &LRegion::EmptyRegion();
}

const LRegion *LLayerView::inputRegion() const noexcept
{
    return m_inputRegion.get();
}

void LLayerView::paintEvent(const PaintEventParams &) noexcept
{
    /* It is not renderable */
}
