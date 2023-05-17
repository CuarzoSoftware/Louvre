#include <private/LCursorPrivate.h>
#include <private/LOutputPrivate.h>
#include <private/LCompositorPrivate.h>
#include <private/LXCursorPrivate.h>

#include <LTexture.h>
#include <LRect.h>
#include <LPainter.h>

#include <X11/Xcursor/Xcursor.h>
#include <string.h>

#include <other/cursor.h>
#include <other/lodepng.h>

using namespace Louvre;

LCursor::LCursor(LOutput *output)
{
    m_imp = new LCursorPrivate();
    imp()->cursor = this;
    setOutput(output);
    imp()->defaultTexture = new LTexture(1);
    imp()->defaultTexture->setDataB(64,64,louvre_default_cursor_data());
    setSizeS(LSize(24));
    useDefault();
}

LCursor::~LCursor()
{
    delete m_imp;
}

LXCursor *LCursor::loadXCursorB(const char *cursor, const char *theme, Int32 suggestedSize, GLuint textureUnit)
{
    XcursorImage *x11Cursor =  XcursorLibraryLoadImage(cursor,theme,suggestedSize);

    if(!x11Cursor)
        return nullptr;

    LXCursor *newCursor = new LXCursor();
    newCursor->imp()->hotspotB.setX(x11Cursor->xhot);
    newCursor->imp()->hotspotB.setY(x11Cursor->yhot);
    newCursor->imp()->texture = new LTexture(textureUnit);
    newCursor->imp()->texture->setDataB(x11Cursor->width, x11Cursor->height, x11Cursor->pixels);

    XcursorImageDestroy(x11Cursor);

    return newCursor;
}

void LCursor::useDefault()
{
    if(!output())
        return;

    if(texture() == imp()->defaultTexture)
        return;

    setTextureB(imp()->defaultTexture, LPointF(9));
}


void LCursor::setTextureB(LTexture *texture, const LPointF &hotspot)
{
    imp()->texture = texture;
    imp()->hotspotB = hotspot;
    imp()->update();
    /* TODO:
    compositor()->imp()->graphicBackend->setCursorTexture(
                imp()->output,
                imp()->texture,
                imp()->sizeS*output()->scale());*/
}

void LCursor::setOutput(LOutput *output)
{
    if(output == imp()->output)
        return;

    //if(imp()->output)
        //setVisible(false);

    imp()->output = output;
    //setVisible(true);
}

void LCursor::moveC(float x, float y)
{
    setPosC(imp()->posC + LPointF(x,y));
}

void Louvre::LCursor::setPosC(const LPointF &pos)
{
    if(!output())
        return;

    for(LOutput *output : compositor()->outputs())
    {
        if(output->rectC().containsPoint(pos) && output)
            setOutput(output);
    }

    imp()->posC = pos;

    LRect area = output()->rectC();
    if(imp()->posC.x() > area.x() + area.w())
        imp()->posC.setX(area.x() + area.w());
    if(imp()->posC.x() < area.x())
        imp()->posC.setX(area.x());

    if(imp()->posC.y() > area.y() + area.h())
        imp()->posC.setY(area.y() + area.h());
    if(imp()->posC.y() < area.y())
        imp()->posC.setY(area.y());


    imp()->update();
}

void LCursor::setHotspotB(const LPointF &hotspot)
{
    imp()->hotspotB = hotspot;
    imp()->update();
}

void LCursor::setSizeS(const LSizeF &size)
{
    if(!imp()->output)
        return;

    imp()->sizeS = size;
    imp()->update();
}

void LCursor::setVisible(bool state)
{
    if(state == visible())
        return;

    imp()->isVisible = state;

    if(!visible())
    {
        for(LOutput *o : compositor()->outputs())
        compositor()->imp()->graphicBackend->setCursorTexture(
                    o,
                    nullptr,
                    imp()->sizeS);
    }
    else if(texture())
    {
        imp()->update();
        /* TODO:
        for(LOutput *o : compositor()->outputs())

        compositor()->imp()->graphicBackend->setCursorTexture(
                    o,
                    texture(),
                    imp()->sizeS*o->scale());*/
    }

}

void LCursor::repaintOutputs()
{
    for(LOutput *o : intersectedOutputs())
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

LCompositor *LCursor::compositor() const
{
    if(output())
        return output()->compositor();
    return nullptr;
}

LOutput *LCursor::output() const
{
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

LCursor::LCursorPrivate *LCursor::imp() const
{
    return m_imp;
}


void LCursor::LCursorPrivate::update()
{
    if(!isVisible || !output || !texture)
        return;

    LPointF newHotspotS;
    newHotspotS = (hotspotB*sizeS)/LSizeF(texture->sizeB());

    LPointF newPosG = posC - (newHotspotS * output->compositor()->globalScale());

    rectC.setPos(newPosG);
    rectC.setSize(sizeS * output->compositor()->globalScale());

    for(LOutput *o : output->compositor()->outputs())
    {
        if(o->rectC().intersects(rectC))
        {
            bool found = (std::find(intersectedOutputs.begin(), intersectedOutputs.end(), o) != intersectedOutputs.end());

            if(!found)
            {
                intersectedOutputs.push_back(o);
                /* TODO:
                cursor->compositor()->imp()->graphicBackend->setCursorTexture(
                            o,
                            texture,
                            sizeS*o->scale());*/
            }
        }
        else
        {
            intersectedOutputs.remove(o);
            cursor->compositor()->imp()->graphicBackend->setCursorTexture(
                        o,
                        nullptr,
                        sizeS);
        }

        if(cursor->hasHardwareSupport(o))
        {
            LPointF p = newPosG - o->posC();
            /*TODO: output->compositor()->imp()->graphicBackend->setCursorPosition(o, (p*o->scale())/o->compositor()->globalScale());*/
        }
    }

}

void LCursor::LCursorPrivate::globalScaleChanged(Int32 oldScale, Int32 newScale)
{
    posC = (posC*newScale)/oldScale;

    if(!output)
        return;

    update();
}



