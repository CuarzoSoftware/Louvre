#include <CZ/Louvre/Private/LCursorPrivate.h>
#include <CZ/Louvre/Private/LOutputPrivate.h>
#include <CZ/Louvre/Private/LCompositorPrivate.h>
#include <CZ/Louvre/Private/LTexturePrivate.h>
#include <CZ/Louvre/Private/LPainterPrivate.h>

#include <CZ/Utils/CZRegionUtils.h>
#include <CZ/Louvre/LXCursor.h>
#include <CZ/Louvre/Roles/LCursorRole.h>
#include <CZ/Louvre/LPointer.h>
#include <CZ/Louvre/LSeat.h>
#include <CZ/skia/core/SkRect.h>
#include <CZ/Louvre/LPainter.h>
#include <CZ/Louvre/LLog.h>

#include <CZ/Louvre/Other/cursor.h>

using namespace Louvre;

bool LCursor::enabled(LOutput *output) const noexcept
{
    if (!output)
        return false;

    return output->imp()->stateFlags.has(LOutput::LOutputPrivate::CursorEnabled);
}

void LCursor::enable(LOutput *output, bool enabled) noexcept
{
    if (!output || output->imp()->stateFlags.has(LOutput::LOutputPrivate::CursorEnabled) == enabled)
        return;

    output->imp()->stateFlags.setFlag(LOutput::LOutputPrivate::CursorEnabled, enabled);
    imp()->textureChanged = true;
    imp()->update();
}

void LCursor::enableHwCompositing(LOutput *output, bool enabled) noexcept
{
    if (!output || output->imp()->stateFlags.has(LOutput::LOutputPrivate::HwCursorEnabled) == enabled)
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

    return hasHardwareSupport(output) && output->imp()->stateFlags.has(LOutput::LOutputPrivate::HwCursorEnabled);
}

const SkRegion &LCursor::damage(LOutput *output) const noexcept
{
    if (!output)
        return CZRegionUtils::Empty();

    return output->imp()->cursorDamage;
}

LCursor::LCursor() noexcept : LPRIVATE_INIT_UNIQUE(LCursor)
{
    eglMakeCurrent(compositor()->eglDisplay(), EGL_NO_SURFACE, EGL_NO_SURFACE, compositor()->eglContext());

    compositor()->imp()->cursor = this;

    if (!imp()->louvreTexture.setDataFromMainMemory(
            SkISize(LOUVRE_DEFAULT_CURSOR_WIDTH, LOUVRE_DEFAULT_CURSOR_HEIGHT),
            LOUVRE_DEFAULT_CURSOR_STRIDE,
            DRM_FORMAT_ARGB8888,
            louvre_default_cursor_data()))
        LLog::warning("[LCursor::LCursor] Failed to create default cursor texture.");

    imp()->defaultTexture = &imp()->louvreTexture;
    imp()->defaultHotspotB = SkPoint(9);

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

    setSize(SkSize(24, 24));
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

void LCursor::replaceDefaultB(const LTexture *texture, SkPoint hotspot) noexcept
{
    if (compositor()->state() == LCompositor::Uninitializing)
        return;

    const bool update { imp()->defaultTexture == imp()->texture };

    if (!texture)
    {
        imp()->defaultTexture = &imp()->louvreTexture;
        imp()->defaultHotspotB = SkPoint(9);
    }
    else
    {
        imp()->defaultTexture = (LTexture*)texture;
        imp()->defaultHotspotB = hotspot;
    }

    if (update)
        useDefault();
}

void LCursor::setTextureB(const LTexture *texture, SkPoint hotspot) noexcept
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
        setTextureB(clientCursor.cursorRole()->surface()->texture(),
                    SkPoint::Make(clientCursor.cursorRole()->hotspotB().x(), clientCursor.cursorRole()->hotspotB().y()));
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
        setTextureB(xcursor->texture(),
                    SkPoint::Make(xcursor->hotspotB().x(), xcursor->hotspotB().y()));
}

const LClientCursor *LCursor::clientCursor() const noexcept
{
    return imp()->clientCursor;
}

void LCursor::move(Float32 x, Float32 y) noexcept
{
    setPos(m_pos + SkPoint(x,y));
}

void LCursor::move(SkPoint delta) noexcept
{
    setPos(m_pos + delta);
}

void Louvre::LCursor::setPos(SkPoint pos) noexcept
{
    for (LOutput *output : compositor()->outputs())
        if (output->rect().contains(pos.x(), pos.y()) && output)
            imp()->setOutput(output);

    if (!cursor()->output())
        return;

    m_pos = pos;

    SkIRect area { cursor()->output()->rect() };
    if (m_pos.x() > area.x() + area.width())
        m_pos.fX = area.x() + area.width();
    if (m_pos.x() < area.x())
        m_pos.fX = area.x();

    if (m_pos.y() > area.y() + area.height())
        m_pos.fY = area.y() + area.height();
    if (m_pos.y() < area.y())
        m_pos.fY = area.y();

    imp()->update();
}

void LCursor::setPos(Float32 x, Float32 y) noexcept
{
    setPos(SkPoint(x, y));
}

void LCursor::setHotspotB(SkPoint hotspot) noexcept
{
    if (imp()->hotspotB != hotspot)
    {
        imp()->hotspotB = hotspot;
        imp()->update();
    }
}

void LCursor::setSize(SkSize size) noexcept
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

SkPoint LCursor::hotspotB() const noexcept
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

SkPoint LCursor::defaultHotspotB() const noexcept
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

const SkIRect &LCursor::rect() const noexcept
{
    return imp()->rect;
}
