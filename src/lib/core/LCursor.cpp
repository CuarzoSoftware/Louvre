#include <private/LCursorPrivate.h>
#include <private/LOutputPrivate.h>
#include <private/LCompositorPrivate.h>
#include <private/LTexturePrivate.h>
#include <private/LPainterPrivate.h>

#include <LXCursor.h>
#include <LCursorRole.h>
#include <LPointer.h>
#include <LSeat.h>
#include <LRect.h>
#include <LPainter.h>
#include <LLog.h>

#include <other/cursor.h>

using namespace Louvre;

bool LCursor::enabled(LOutput *output) const noexcept
{
    if (!output)
        return false;

    return output->imp()->stateFlags.check(LOutput::LOutputPrivate::CursorEnabled);
}

void LCursor::enable(LOutput *output, bool enabled) noexcept
{
    if (!output || output->imp()->stateFlags.check(LOutput::LOutputPrivate::CursorEnabled) == enabled)
        return;

    output->imp()->stateFlags.setFlag(LOutput::LOutputPrivate::CursorEnabled, enabled);
    imp()->textureChanged = true;
    imp()->update();
}

void LCursor::enableHwCompositing(LOutput *output, bool enabled) noexcept
{
    if (!output || output->imp()->stateFlags.check(LOutput::LOutputPrivate::HwCursorEnabled) == enabled)
        return;

    output->imp()->stateFlags.setFlag(LOutput::LOutputPrivate::HwCursorEnabled, enabled);
    imp()->textureChanged = true;
    imp()->update();
    output->repaint();
}

bool LCursor::hwCompositingEnabled(LOutput *output) const noexcept
{
    if (!output)
        return false;

    return hasHardwareSupport(output) && output->imp()->stateFlags.check(LOutput::LOutputPrivate::HwCursorEnabled);
}

const LRegion &LCursor::damage(LOutput *output) const noexcept
{
    if (!output)
        return LRegion::EmptyRegion();

    return output->imp()->cursorDamage;
}

LCursor::LCursor() noexcept : LPRIVATE_INIT_UNIQUE(LCursor)
{
    compositor()->imp()->cursor = this;

    if (!imp()->louvreTexture.setDataFromMainMemory(
            LSize(LOUVRE_DEFAULT_CURSOR_WIDTH, LOUVRE_DEFAULT_CURSOR_HEIGHT),
            LOUVRE_DEFAULT_CURSOR_STRIDE,
            DRM_FORMAT_ARGB8888,
            louvre_default_cursor_data()))
        LLog::warning("[LCursor::LCursor] Failed to create default cursor texture.");

    imp()->defaultTexture = &imp()->louvreTexture;
    imp()->defaultHotspotB = LPointF(9);

    glGenFramebuffers(1, &imp()->glFramebuffer);

    if (imp()->glFramebuffer == 0)
    {
        LLog::error("[LCursor::LCursor] Failed to create GL framebuffer.");
        imp()->hasFb = false;
        goto skipGL;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, imp()->glFramebuffer);
    glGenRenderbuffers(1, &imp()->glRenderbuffer);

    if (imp()->glRenderbuffer == 0)
    {
        LLog::error("[LCursor::LCursor] Failed to create GL renderbuffer.");
        imp()->hasFb = false;
        glDeleteFramebuffers(1, &imp()->glFramebuffer);
        goto skipGL;
    }

    glBindRenderbuffer(GL_RENDERBUFFER, imp()->glRenderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, LOUVRE_DEFAULT_CURSOR_WIDTH, LOUVRE_DEFAULT_CURSOR_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, imp()->glRenderbuffer);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        LLog::error("[LCursor::LCursor] GL_FRAMEBUFFER incomplete.");
        imp()->hasFb = false;
        glDeleteRenderbuffers(1, &imp()->glRenderbuffer);
        glDeleteFramebuffers(1, &imp()->glFramebuffer);
    }

    skipGL:

    setSize(LSize(24));
    useDefault();
    setVisible(true);
}

LCursor::~LCursor()
{
    notifyDestruction();

    compositor()->imp()->cursor = nullptr;

    if (imp()->glRenderbuffer)
        glDeleteRenderbuffers(1, &imp()->glRenderbuffer);

    if (imp()->glFramebuffer)
        glDeleteFramebuffers(1, &imp()->glFramebuffer);
}

void LCursor::useDefault() noexcept
{
    if (compositor()->state() == LCompositor::Uninitializing)
        return;

    imp()->clientCursor.reset();

    if (imp()->texture == imp()->defaultTexture && imp()->hotspotB == imp()->defaultHotspotB)
        return;

    setTextureB(imp()->defaultTexture, imp()->defaultHotspotB);
}

void LCursor::replaceDefaultB(const LTexture *texture, const LPointF &hotspot) noexcept
{
    if (compositor()->state() == LCompositor::Uninitializing)
        return;

    const bool update { imp()->defaultTexture == imp()->texture };

    if (!texture)
    {
        imp()->defaultTexture = &imp()->louvreTexture;
        imp()->defaultHotspotB = LPointF(9);
    }
    else
    {
        imp()->defaultTexture = (LTexture*)texture;
        imp()->defaultHotspotB = hotspot;
    }

    if (update)
        useDefault();
}

void LCursor::setTextureB(const LTexture *texture, const LPointF &hotspot) noexcept
{
    imp()->clientCursor.reset();

    if (!texture)
        return;

    if (imp()->texture == texture && imp()->lastTextureSerial == texture->serial() && hotspot == imp()->hotspotB)
        return;

    if (imp()->texture != texture || imp()->lastTextureSerial != texture->serial())
    {
        imp()->texture = (LTexture*)texture;
        imp()->textureChanged = true;
        imp()->lastTextureSerial = texture->serial();
    }

    imp()->hotspotB = hotspot;
    imp()->update();
}

void LCursor::setCursor(const LClientCursor &clientCursor) noexcept
{
    if (clientCursor.cursorRole())
    {
        setTextureB(clientCursor.cursorRole()->surface()->texture(), clientCursor.cursorRole()->hotspotB());
        imp()->clientCursor.reset(&clientCursor);
        setVisible(clientCursor.visible());
    }
    else
    {
        setVisible(clientCursor.visible());
        useDefault();
    }
}

void LCursor::setCursor(const LXCursor *xcursor) noexcept
{
    if (xcursor)
        setTextureB(xcursor->texture(), xcursor->hotspotB());
}

const LClientCursor *LCursor::clientCursor() const noexcept
{
    return imp()->clientCursor;
}

void LCursor::move(Float32 x, Float32 y) noexcept
{
    setPos(m_pos + LPointF(x,y));
}

void LCursor::move(const LPointF &delta) noexcept
{
    setPos(m_pos + delta);
}

void Louvre::LCursor::setPos(const LPointF &pos) noexcept
{
    for (LOutput *output : compositor()->outputs())
        if (output->rect().containsPoint(pos) && output)
            imp()->setOutput(output);

    if (!cursor()->output())
        return;

    m_pos = pos;

    LRect area = cursor()->output()->rect();
    if (m_pos.x() > area.x() + area.w())
        m_pos.setX(area.x() + area.w());
    if (m_pos.x() < area.x())
        m_pos.setX(area.x());

    if (m_pos.y() > area.y() + area.h())
        m_pos.setY(area.y() + area.h());
    if (m_pos.y() < area.y())
        m_pos.setY(area.y());

    imp()->update();
}

void LCursor::setPos(Float32 x, Float32 y) noexcept
{
    setPos(LPointF(x, y));
}

void LCursor::setHotspotB(const LPointF &hotspot) noexcept
{
    if (imp()->hotspotB != hotspot)
    {
        imp()->hotspotB = hotspot;
        imp()->update();
    }
}

void LCursor::setSize(const LSizeF &size) noexcept
{
    if (imp()->size != size)
    {
        imp()->size = size;
        imp()->textureChanged = true;
        imp()->update();
    }
}

void LCursor::setVisible(bool state) noexcept
{
    if (state == visible())
        return;

    imp()->isVisible = state;

    if (!visible())
    {
        for (LOutput *o : compositor()->outputs())
            compositor()->imp()->graphicBackend->outputSetCursorTexture(
                        o,
                        nullptr);
    }
    else if (texture())
    {
        imp()->textureChanged = true;
        imp()->update();
    }
}

void LCursor::repaintOutputs(bool nonHardwareOnly) noexcept
{
    for (LOutput *o : intersectedOutputs())
        if (!nonHardwareOnly || !hwCompositingEnabled(o))
            o->repaint();

    if (clientCursor() && clientCursor()->cursorRole() && clientCursor()->cursorRole()->surface())
    {
        for (LOutput *output : compositor()->outputs())
        {
            if (output == cursor()->output())
                clientCursor()->cursorRole()->surface()->sendOutputEnterEvent(output);
            else
                clientCursor()->cursorRole()->surface()->sendOutputLeaveEvent(output);
        }
    }
}

bool LCursor::visible() const noexcept
{
    return imp()->isVisible;
}

bool LCursor::hasHardwareSupport(const LOutput *output) const noexcept
{
    if (!imp()->hasFb)
        return false;

    return compositor()->imp()->graphicBackend->outputHasHardwareCursorSupport((LOutput*)output);
}

const LPointF &LCursor::hotspotB() const noexcept
{
    return imp()->hotspotB;
}

LTexture *LCursor::texture() const noexcept
{
    return imp()->texture;
}

LTexture *LCursor::defaultTexture() const noexcept
{
    return imp()->defaultTexture;
}

const LPointF &LCursor::defaultHotspotB() const noexcept
{
    return imp()->defaultHotspotB;
}

LTexture *LCursor::defaultLouvreTexture() const noexcept
{
    return &imp()->louvreTexture;
}

LOutput *LCursor::output() const noexcept
{
    if (m_output)
        return m_output;

    if (!compositor()->outputs().empty())
    {
        m_output = compositor()->outputs().front();
        imp()->textureChanged = true;
    }

    return m_output;
}

const std::vector<LOutput *> &LCursor::intersectedOutputs() const noexcept
{
    return imp()->intersectedOutputs;
}

const LRect &LCursor::rect() const noexcept
{
    return imp()->rect;
}
