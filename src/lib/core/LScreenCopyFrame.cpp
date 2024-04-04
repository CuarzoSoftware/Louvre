#include <protocols/ScreenCopy/wlr-screencopy-unstable-v1.h>
#include <protocols/ScreenCopy/RScreenCopyFrame.h>
#include <protocols/Wayland/GOutput.h>
#include <private/LPainterPrivate.h>
#include <private/LOutputPrivate.h>
#include <LScreenCopyFrame.h>
#include <LUtils.h>
#include <LTime.h>

using namespace Louvre;

bool LScreenCopyFrame::compositeCursor() const noexcept
{
    return m_frame.overlayCursor();
}

LClient *LScreenCopyFrame::client() const noexcept
{
    return m_frame.client();
}

const LRect &LScreenCopyFrame::rect() const noexcept
{
    return m_frame.region();
}

void LScreenCopyFrame::copy(const LRegion *damage) const noexcept
{
    if (m_frame.m_handled)
        return;

    if (!m_frame.m_bufferContainer.m_buffer)
    {
        m_frame.failed();
        return;
    }

    wl_shm_buffer *shm_buffer = wl_shm_buffer_get(m_frame.m_bufferContainer.m_buffer);
    Int32 width = wl_shm_buffer_get_width(shm_buffer);
    Int32 height = wl_shm_buffer_get_height(shm_buffer);

    if (width != m_output.get()->realBufferSize().w() || height != m_output.get()->realBufferSize().h())
    {
        m_frame.failed();
        return;
    }

    wl_shm_buffer_begin_access(shm_buffer);
    UInt8 *pixels = (UInt8*)wl_shm_buffer_get_data(shm_buffer);

    timespec t = LTime::ns();

    glPixelStorei(GL_PACK_ALIGNMENT, 4);
    glPixelStorei(GL_PACK_ROW_LENGTH, width);
    glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_PACK_SKIP_ROWS, 0);
    glReadPixels(0, 0, width, height, m_output.get()->painter()->imp()->openGLExtensions.EXT_read_format_bgra ? GL_BGRA : GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    wl_shm_buffer_end_access(shm_buffer);

    if (!m_output.get()->usingFractionalScale() || !m_output.get()->fractionalOversamplingEnabled())
        m_frame.flags(ZWLR_SCREENCOPY_FRAME_V1_FLAGS_Y_INVERT);
    m_frame.ready(t.tv_sec >> 32, t.tv_sec & 0xffffffff, t.tv_nsec);
}

void LScreenCopyFrame::reject() const noexcept
{
    m_frame.failed();
}

LScreenCopyFrame::LScreenCopyFrame(Protocols::ScreenCopy::RScreenCopyFrame &frame) noexcept :
    m_frame(frame),
    m_output(frame.outputRes()->output())
{}

LScreenCopyFrame::~LScreenCopyFrame() noexcept
{
    if (m_output.get())
        LVectorRemoveOneUnordered(m_output.get()->imp()->screenCopyFrames, this);
}
