#include <LNinePatchView.h>
#include <LTexture.h>

using namespace Louvre;

enum Edges { L, T, R, B, TL, TR, BR, BL, C, LAST };

LNinePatchView::LNinePatchView(LView *parent) noexcept : LView(NinePatchType, false, parent)
{
    init(nullptr, 1, {0});
}

LNinePatchView::LNinePatchView(LTexture *texture, Float32 bufferScale, const LRectF &center, LView *parent) noexcept :
    LView(NinePatchType, false, parent)
{
    init(texture, bufferScale, center);
}

void LNinePatchView::init(LTexture *texture, Float32 bufferScale, const LRectF &center) noexcept
{
    for (size_t i = 0; i < LAST; i++)
    {
        m_subViews[i].setParent(this);
        m_subViews[i].enableSrcRect(true);
        m_subViews[i].enableDstSize(true);
        m_subViews[i].setInputRegion(&LRegion::EmptyRegion());
    }

    m_texture.setOnDestroyCallback([this](auto) {
        setTexture(nullptr, 1.f, {0.f});
    });

    setTexture(texture, bufferScale, center);
}

void LNinePatchView::updateSubViews() noexcept
{
    // Update corners pos
    m_subViews[TL].setPos(0, 0);
    m_subViews[TR].setPos(nativeSize().w() - m_subViews[TR].nativeSize().w(), 0);
    m_subViews[BR].setPos(
        nativeSize().w() - m_subViews[BR].nativeSize().w(),
        nativeSize().h() - m_subViews[BR].nativeSize().h());
    m_subViews[BL].setPos(0, nativeSize().h() - m_subViews[BL].nativeSize().h());

    // Stretch/position edges

    m_subViews[L].setPos(0, m_center.y());
    m_subViews[L].setDstSize(
        m_subViews[L].nativeSize().w(), // Keep the same width
        nativeSize().h() - m_subViews[TL].nativeSize().h() - m_subViews[BL].nativeSize().h());

    m_subViews[T].setPos(m_subViews[TL].nativeSize().w(), 0);
    m_subViews[T].setDstSize(
        nativeSize().w() - m_subViews[TL].nativeSize().w() * 2,
        m_subViews[T].nativeSize().h()); // Keep the same height

    m_subViews[R].setPos(nativeSize().w() - m_subViews[R].nativeSize().w(), m_center.y());
    m_subViews[R].setDstSize(
        m_subViews[R].nativeSize().w(), // Keep the same width
        nativeSize().h() - m_subViews[TR].nativeSize().h() - m_subViews[BR].nativeSize().h());

    m_subViews[B].setPos(m_center.x(), nativeSize().h() - m_subViews[B].nativeSize().h());
    m_subViews[B].setDstSize(
        nativeSize().w() - m_subViews[TL].nativeSize().w() - m_subViews[TR].nativeSize().w(),
        m_subViews[B].nativeSize().h()); // Keep the same height

    m_subViews[C].setPos(m_subViews[TL].nativeSize());
    m_subViews[C].setDstSize(
        m_subViews[T].nativeSize().w(),
        m_subViews[L].nativeSize().h());
}

void LNinePatchView::setPos(Int32 x, Int32 y) noexcept
{
    if (x == m_nativePos.x() && y == m_nativePos.y())
        return;

    m_nativePos.setX(x);
    m_nativePos.setY(y);

    if (!repaintCalled() && mapped())
        repaint();
}

void LNinePatchView::setPos(const LPoint &pos) noexcept
{
    setPos(pos.x(), pos.y());
}

void LNinePatchView::setSize(Int32 width, Int32 height) noexcept
{
    if (width < m_minSize.w())
        width = m_minSize.w();

    if (height < m_minSize.h())
        height = m_minSize.h();

    if (width == m_nativeSize.w() && height == m_nativeSize.h())
        return;

    m_nativeSize.setW(width);
    m_nativeSize.setH(height);
    updateSubViews();
}

void LNinePatchView::setSize(const LSize &size) noexcept
{
    setSize(size.w(), size.h());
}

const LSize &LNinePatchView::minSize() const noexcept
{
    return m_minSize;
}

LTexture *LNinePatchView::texture() const noexcept
{
    return m_texture;
}

void LNinePatchView::setTexture(LTexture *texture, Float32 bufferScale, const LRectF &center) noexcept
{
    if (m_texture.get() != texture)
    {
        m_texture.reset(texture);
        repaint();
    }

    for (size_t i = 0; i < LAST; i++)
    {
        m_subViews[i].setTexture(texture);
        m_subViews[i].setBufferScale(bufferScale);
    }

    if (!m_texture || center.w() <= 0 || center.h() <= 0)
    {
        m_minSize = {0};
        m_center = {0.f};
        return;
    }

    m_minSize = texture->sizeB()/bufferScale;

    if (m_nativeSize.w() < m_minSize.w())
        m_nativeSize.setW(m_minSize.w());

    if (m_nativeSize.h() < m_minSize.h())
        m_nativeSize.setH(m_minSize.h());

    Float32 x0 { center.x() };
    Float32 x1 { center.x() + center.w() };
    Float32 y0 { center.y() };
    Float32 y1 { center.y() + center.h() };

    if (x0 < 0.f) x0 = 0.f;
    else if (x0 > m_minSize.w()) x0 = m_minSize.w();

    if (y0 < 0.f) y0 = 0.f;
    else if (y0 > m_minSize.h()) y0 = m_minSize.h();

    if (x1 > m_minSize.w()) x1 = m_minSize.w();
    else if (x1 < x0) x1 = x0;

    if (y1 > m_minSize.h()) y1 = m_minSize.h();
    else if (y1 < y0) y1 = y0;

    m_center = {x0, y0, x1 - x0, y1 - y0 };

    m_subViews[C].setSrcRect(m_center);

    m_subViews[L].setSrcRect(LRectF(0, m_center.y(), m_center.x(), m_center.h()));
    m_subViews[L].setDstSize(m_subViews[L].srcRect().size());

    m_subViews[T].setSrcRect(LRectF(m_center.x(), 0, m_center.w(), m_center.y()));
    m_subViews[T].setDstSize(m_subViews[T].srcRect().size());

    m_subViews[R].setSrcRect(LRectF(
        m_center.x() + m_center.w(),
        m_center.y(),
        m_minSize.w() - m_center.x() - m_center.w(),
        m_center.h()));
    m_subViews[R].setDstSize(m_subViews[R].srcRect().size());

    m_subViews[B].setSrcRect(LRectF(
        m_center.x(),
        m_center.y() + m_center.h(),
        m_center.w(),
        m_minSize.h() - m_center.y() - m_center.h()));
    m_subViews[B].setDstSize(m_subViews[B].srcRect().size());

    m_subViews[TL].setSrcRect(LRectF(0, 0, m_center.x(), m_center.y()));
    m_subViews[TL].setDstSize(m_subViews[TL].srcRect().size());

    m_subViews[TR].setSrcRect(LRectF(
        m_center.x() + m_center.w(),
        0,
        m_minSize.w() - m_center.x() - m_center.w(),
        m_center.y()));
    m_subViews[TR].setDstSize(m_subViews[TR].srcRect().size());

    m_subViews[BR].setSrcRect(LRectF(
        m_center.x() + m_center.w(),
        m_center.y() + m_center.h(),
        m_minSize.w() - m_center.x() - m_center.w(),
        m_minSize.h() - m_center.y() - m_center.h()));
    m_subViews[BR].setDstSize(m_subViews[BR].srcRect().size());

    m_subViews[BL].setSrcRect(LRectF(
        0,
        m_center.y() + m_center.h(),
        m_center.x(),
        m_minSize.h() - m_center.y() - m_center.h()));
    m_subViews[BL].setDstSize(m_subViews[BL].srcRect().size());

    updateSubViews();
}

const LRectF &LNinePatchView::center() const noexcept
{
    return m_center;
}

LTextureView *LNinePatchView::getSubView(LBitset<LEdge> edge) noexcept
{
    switch (edge.get())
    {
    case LEdgeLeft:
        return &m_subViews[L];
    case LEdgeTop:
        return &m_subViews[T];
    case LEdgeRight:
        return &m_subViews[R];
    case LEdgeBottom:
        return &m_subViews[B];
    case LEdgeTop | LEdgeLeft:
        return &m_subViews[TL];
    case LEdgeTop | LEdgeRight:
        return &m_subViews[TR];
    case LEdgeBottom | LEdgeRight:
        return &m_subViews[BR];
    case LEdgeBottom | LEdgeLeft:
        return &m_subViews[BL];
    default:
        return &m_subViews[C];
    }
}

std::array<LTextureView, 9> &LNinePatchView::subViews() noexcept
{
    return m_subViews;
}

bool LNinePatchView::nativeMapped() const noexcept
{
    return m_texture != nullptr;
}

const LPoint &LNinePatchView::nativePos() const noexcept
{
    return m_nativePos;
}

const LSize &LNinePatchView::nativeSize() const noexcept
{
    return m_nativeSize;
}

Float32 LNinePatchView::bufferScale() const noexcept
{
    return m_subViews[0].bufferScale();
}

void LNinePatchView::enteredOutput(LOutput *output) noexcept
{
    LVectorPushBackIfNonexistent(m_outputs, output);
}

void LNinePatchView::leftOutput(LOutput *output) noexcept
{
    LVectorRemoveOneUnordered(m_outputs, output);
}

const std::vector<LOutput *> &LNinePatchView::outputs() const noexcept
{
    return m_outputs;
}

void LNinePatchView::requestNextFrame(LOutput *output) noexcept
{
    L_UNUSED(output);
}

const LRegion *LNinePatchView::damage() const noexcept
{
    return &LRegion::EmptyRegion();
}

const LRegion *LNinePatchView::translucentRegion() const noexcept
{
    return &LRegion::EmptyRegion();
}

const LRegion *LNinePatchView::opaqueRegion() const noexcept
{
    return &LRegion::EmptyRegion();
}

const LRegion *LNinePatchView::inputRegion() const noexcept
{
    return m_inputRegion.get();
}

void LNinePatchView::paintEvent(const PaintEventParams &params) noexcept
{
    L_UNUSED(params);

    /* It is not renderable */
}
