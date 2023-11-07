#include <private/LCursorPrivate.h>
#include <private/LOutputPrivate.h>
#include <private/LCompositorPrivate.h>
#include <private/LXCursorPrivate.h>
#include <private/LTexturePrivate.h>
#include <private/LPainterPrivate.h>

#include <LRect.h>
#include <LPainter.h>
#include <LLog.h>

#include <string.h>

#include <other/cursor.h>

#include <algorithm>

using namespace Louvre;

#define L_CURSOR_WIDTH 64
#define L_CURSOR_HEIGHT 64
#define L_CURSOR_BPP 32
#define L_CURSOR_STRIDE L_CURSOR_WIDTH*(L_CURSOR_BPP/8)

LCursor::LCursor()
{
    m_imp = new LCursorPrivate();
    compositor()->imp()->cursor = this;
    imp()->louvreTexture = new LTexture();

    if (!imp()->louvreTexture->setDataB(LSize(L_CURSOR_WIDTH, L_CURSOR_HEIGHT), L_CURSOR_STRIDE, DRM_FORMAT_ABGR8888, louvre_default_cursor_data()))
        LLog::warning("[compositor] Could not create default cursor texture.");

    imp()->defaultTexture = imp()->louvreTexture;
    imp()->defaultHotspotB = LPointF(9);

    glGenFramebuffers(1, &imp()->glFramebuffer);

    if (imp()->glFramebuffer == 0)
        LLog::error("[cursor] Could not create GL framebuffer.");

    glBindFramebuffer(GL_FRAMEBUFFER, imp()->glFramebuffer);

    glGenRenderbuffers(1, &imp()->glRenderbuffer);

    if (imp()->glRenderbuffer == 0)
        LLog::error("[cursor] Could not create GL renderbuffer.");

    glBindRenderbuffer(GL_RENDERBUFFER, imp()->glRenderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, L_CURSOR_WIDTH, L_CURSOR_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, imp()->glRenderbuffer);

    // Check framebuffer completeness
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        LLog::error("[cursor] Framebuffer is not complete.");
    }

    setSize(LSize(24));
    useDefault();
    setVisible(true);
}

LCursor::~LCursor()
{
    delete m_imp;
}

void LCursor::useDefault()
{
    if (compositor()->state() == LCompositor::Uninitializing)
        return;

    if (imp()->texture == imp()->defaultTexture && imp()->hotspotB == imp()->defaultHotspotB)
        return;

    setTextureB(imp()->defaultTexture, imp()->defaultHotspotB);
}

void LCursor::replaceDefaultB(const LTexture *texture, const LPointF &hotspot)
{
    if (compositor()->state() == LCompositor::Uninitializing)
        return;

    bool update = imp()->defaultTexture == imp()->texture;

    if (!texture)
    {
        imp()->defaultTexture = imp()->louvreTexture;
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

void LCursor::setTextureB(const LTexture *texture, const LPointF &hotspot)
{
    if (!texture)
        return;

    if (imp()->texture == texture && imp()->lastTextureSerial == texture->imp()->serial && hotspot == imp()->hotspotB)
        return;

    if (imp()->texture != texture || imp()->lastTextureSerial != texture->imp()->serial)
    {
        imp()->texture = (LTexture*)texture;
        imp()->textureChanged = true;
        imp()->lastTextureSerial = texture->imp()->serial;
    }

    imp()->hotspotB = hotspot;
    imp()->update();
}

void LCursor::move(Float32 x, Float32 y)
{
    imp()->setPos(imp()->pos + LPointF(x,y));
}

void Louvre::LCursor::setPos(const LPointF &pos)
{
    imp()->setPos(pos);
}

void LCursor::setPos(Float32 x, Float32 y)
{
    setPos(LPointF(x, y));
}

void LCursor::setHotspotB(const LPointF &hotspot)
{
    if (imp()->hotspotB != hotspot)
    {
        imp()->hotspotB = hotspot;
        imp()->update();
    }
}

void LCursor::setSize(const LSizeF &size)
{
    if (imp()->size != size)
    {
        imp()->size = size;
        imp()->textureChanged = true;
        imp()->update();
    }
}

void LCursor::setVisible(bool state)
{
    if (state == visible())
        return;

    imp()->isVisible = state;

    if (!visible())
    {
        for (LOutput *o : compositor()->outputs())
            compositor()->imp()->graphicBackend->setCursorTexture(
                        o,
                        nullptr);
    }
    else if (texture())
    {
        imp()->textureChanged = true;
        imp()->update();
    }
}

void LCursor::repaintOutputs(bool nonHardwareOnly)
{
    for (LOutput *o : intersectedOutputs())
        if (!nonHardwareOnly || !hasHardwareSupport(o))
            o->repaint();

    if (seat()->pointer()->lastCursorRequest())
    {
        for (LOutput *output : compositor()->outputs())
        {
            if (output == cursor()->output())
                seat()->pointer()->lastCursorRequest()->surface()->sendOutputEnterEvent(output);
            else
                seat()->pointer()->lastCursorRequest()->surface()->sendOutputLeaveEvent(output);
        }
    }
}

bool LCursor::visible() const
{
    return imp()->isVisible;
}

bool LCursor::hasHardwareSupport(const LOutput *output) const
{
    return compositor()->imp()->graphicBackend->hasHardwareCursorSupport((LOutput*)output);
}

const LPointF &LCursor::pos() const
{
    return imp()->pos;
}

const LPointF &LCursor::hotspotB() const
{
    return imp()->hotspotB;
}

LTexture *LCursor::texture() const
{
    return imp()->texture;
}

LTexture *LCursor::defaultTexture() const
{
    return imp()->defaultTexture;
}

const LPointF &LCursor::defaultHotspotB() const
{
    return imp()->defaultHotspotB;
}

LTexture *LCursor::defaultLouvreTexture() const
{
    return imp()->louvreTexture;
}

LOutput *LCursor::output() const
{
    if (imp()->output)
        return imp()->output;

    if (!compositor()->outputs().empty())
    {
        imp()->output = compositor()->outputs().front();
        imp()->textureChanged = true;
    }

    return imp()->output;
}

const std::list<LOutput *> &LCursor::intersectedOutputs() const
{
    return imp()->intersectedOutputs;
}

const LRect &LCursor::rect() const
{
    return imp()->rect;
}
