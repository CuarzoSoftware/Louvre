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
#include <other/lodepng.h>

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
    imp()->defaultTexture = new LTexture(1);

    if (!imp()->defaultTexture->setDataB(LSize(L_CURSOR_WIDTH, L_CURSOR_HEIGHT), L_CURSOR_STRIDE, DRM_FORMAT_ABGR8888, louvre_default_cursor_data()))
        LLog::warning("[compositor] Could not create default cursor texture.");

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

    setSizeS(LSize(24));
    useDefault();
    setVisible(true);
}

LCursor::~LCursor()
{
    delete m_imp;
}

void LCursor::useDefault()
{
    setTextureB(imp()->defaultTexture, LPointF(9));
}

static void texture2Buffer(LCursor *cursor, const LSizeF &size)
{
    glBindFramebuffer(GL_FRAMEBUFFER, cursor->imp()->glFramebuffer);
    cursor->compositor()->imp()->painter->imp()->scaleCursor(
        cursor->texture(),
        LRect(0, 0, cursor->texture()->sizeB().w(), -cursor->texture()->sizeB().h()),
        LRect(0, size));

    glReadPixels(0, 0, 64, 64, GL_RGBA , GL_UNSIGNED_BYTE, cursor->imp()->buffer);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void LCursor::setTextureB(const LTexture *texture, const LPointF &hotspot)
{
    if (!texture)
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

void LCursor::setOutput(LOutput *output)
{
    bool update = false;

    if (!imp()->output)
        update = true;

    imp()->output = output;

    if (update)
    {
        imp()->textureChanged = true;
        imp()->update();
    }
}

void LCursor::moveC(float x, float y)
{
    setPosC(imp()->posC + LPointF(x,y));
}

void Louvre::LCursor::setPosC(const LPointF &pos)
{
    for (LOutput *output : compositor()->outputs())
        if (output->rectC().containsPoint(pos) && output)
            setOutput(output);

    if (!output())
        return;

    imp()->posC = pos;

    LRect area = output()->rectC();
    if (imp()->posC.x() > area.x() + area.w())
        imp()->posC.setX(area.x() + area.w());
    if (imp()->posC.x() < area.x())
        imp()->posC.setX(area.x());

    if (imp()->posC.y() > area.y() + area.h())
        imp()->posC.setY(area.y() + area.h());
    if (imp()->posC.y() < area.y())
        imp()->posC.setY(area.y());

    imp()->update();
}

void LCursor::setHotspotB(const LPointF &hotspot)
{
    if (imp()->hotspotB != hotspot)
    {
        imp()->hotspotB = hotspot;
        imp()->update();
    }
}

void LCursor::setSizeS(const LSizeF &size)
{    
    if (imp()->sizeS != size)
    {
        imp()->sizeS = size;
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

void LCursor::repaintOutputs()
{
    for (LOutput *o : intersectedOutputs())
        o->repaint();
}

bool LCursor::visible() const
{
    return imp()->isVisible;
}

bool LCursor::hasHardwareSupport(const LOutput *output) const
{
    return compositor()->imp()->graphicBackend->hasHardwareCursorSupport((LOutput*)output);
}

const LPointF &LCursor::posC() const
{
    return imp()->posC;
}

const LPointF &LCursor::hotspotB() const
{
    return imp()->hotspotB;
}

LTexture *LCursor::texture() const
{
    return imp()->texture;
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

const LRect &LCursor::rectC() const
{
    return imp()->rectC;
}

void LCursor::LCursorPrivate::update()
{
    if (!cursor()->output())
        return;

    LPointF newHotspotS;
    newHotspotS = (hotspotB*sizeS)/LSizeF(texture->sizeB());

    LPointF newPosC = posC - (newHotspotS * compositor()->globalScale());

    rectC.setPos(newPosC);
    rectC.setSize(sizeS * compositor()->globalScale());

    for (LOutput *o : compositor()->outputs())
    {
        if (o->rectC().intersects(rectC))
        {
            bool found = (std::find(intersectedOutputs.begin(), intersectedOutputs.end(), o) != intersectedOutputs.end());

            if (!found)
            {
                textureChanged = true;
                intersectedOutputs.push_back(o);
            }
        }
        else
            intersectedOutputs.remove(o);

        if (cursor()->hasHardwareSupport(o))
        {
            LPointF p = newPosC - o->posC();
            compositor()->imp()->graphicBackend->setCursorPosition(o, (p*o->scale())/compositor()->globalScale());
        }
    }
}

void LCursor::LCursorPrivate::textureUpdate()
{
    if (!cursor()->output())
        return;

    if (!textureChanged)
        return;

    LPointF newHotspotS;
    newHotspotS = (hotspotB*sizeS)/LSizeF(texture->sizeB());

    LPointF newPosC = posC - (newHotspotS * compositor()->globalScale());
    rectC.setPos(newPosC);
    rectC.setSize(sizeS * compositor()->globalScale());

    for (LOutput *o : compositor()->outputs())
    {
        if (o->rectC().intersects(rectC))
        {
            bool found = (std::find(intersectedOutputs.begin(), intersectedOutputs.end(), o) != intersectedOutputs.end());

            if (!found)
                intersectedOutputs.push_back(o);

            if (cursor()->hasHardwareSupport(o) && (textureChanged || !found))
            {
                texture2Buffer(cursor(), sizeS*o->scale());
                compositor()->imp()->graphicBackend->setCursorTexture(o, buffer);
            }
        }
        else
        {
            intersectedOutputs.remove(o);
            compositor()->imp()->graphicBackend->setCursorTexture(o, nullptr);
        }

        if (cursor()->hasHardwareSupport(o))
        {
            LPointF p = newPosC - o->posC();
            compositor()->imp()->graphicBackend->setCursorPosition(o, (p*o->scale())/compositor()->globalScale());
        }
    }

    textureChanged = false;
}

void LCursor::LCursorPrivate::globalScaleChanged(Int32 oldScale, Int32 newScale)
{
    posC = (posC*newScale)/oldScale;

    textureChanged = true;

    if (!cursor()->output())
        return;

    update();
}
