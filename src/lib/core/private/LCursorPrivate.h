#ifndef LCURSORPRIVATE_H
#define LCURSORPRIVATE_H

#include <private/LCompositorPrivate.h>
#include <private/LPainterPrivate.h>
#include <private/LOutputPrivate.h>
#include <LCursor.h>
#include <algorithm>

using namespace Louvre;

inline static void texture2Buffer(LCursor *cursor, const LSizeF &size, LFramebuffer::Transform transform);

LPRIVATE_CLASS_NO_COPY(LCursor)
    LCursorPrivate();
    LRect rect;
    LPointF pos;
    LPointF hotspotB;
    LSizeF size;
    LOutput *output                                     = nullptr;
    std::list<LOutput*>intersectedOutputs;
    bool isVisible                                      = true;

    UInt32 lastTextureSerial                            = 0;
    bool textureChanged                                 = false;
    bool posChanged                                     = false;
    bool hasFb                                          = true;
    LTexture *texture                                   = nullptr;
    LPointF defaultHotspotB;
    LTexture *defaultTexture                            = nullptr;
    LTexture louvreTexture;
    GLuint glFramebuffer, glRenderbuffer;
    UChar8 buffer[64*64*4];

    inline void setOutput(LOutput *out)
    {
        bool up = false;

        if (!output)
            up = true;

        output = out;

        if (up)
        {
            textureChanged = true;
            update();
        }
    }

    inline void setPos(const LPointF &pos)
    {
        for (LOutput *output : LCompositor::compositor()->outputs())
            if (output->rect().containsPoint(pos) && output)
                setOutput(output);

        if (!cursor()->output())
            return;

        this->pos = pos;

        LRect area = cursor()->output()->rect();
        if (this->pos.x() > area.x() + area.w())
            this->pos.setX(area.x() + area.w());
        if (this->pos.x() < area.x())
            this->pos.setX(area.x());

        if (this->pos.y() > area.y() + area.h())
            this->pos.setY(area.y() + area.h());
        if (this->pos.y() < area.y())
            this->pos.setY(area.y());

        this->update();
    }

    inline void update()
    {
        if (!cursor()->output())
            return;

        posChanged = true;
        return;

        LPointF newHotspotS;
        newHotspotS = (hotspotB*size)/LSizeF(texture->sizeB());

        LPointF newPosS = pos - newHotspotS;

        rect.setPos(newPosS);
        rect.setSize(size);

        for (LOutput *o : compositor()->outputs())
        {
            if (o->rect().intersects(rect))
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
        }
    }

    // Called once per main loop iteration
    inline void textureUpdate()
    {
        if (!cursor()->output())
            return;

        if (!textureChanged && !posChanged)
            return;

        LPointF newHotspotS;
        newHotspotS = (hotspotB*size)/LSizeF(texture->sizeB());

        LPointF newPosS = pos - newHotspotS;
        rect.setPos(newPosS);
        rect.setSize(size);

        for (LOutput *o : compositor()->outputs())
        {
            if (o->rect().intersects(rect))
            {
                bool found = (std::find(intersectedOutputs.begin(), intersectedOutputs.end(), o) != intersectedOutputs.end());

                if (!found)
                    intersectedOutputs.push_back(o);

                if (cursor()->hasHardwareSupport(o) && (textureChanged || !found))
                {
                    texture2Buffer(cursor(), size * o->fractionalScale(), o->transform());
                    compositor()->imp()->graphicBackend->outputSetCursorTexture(o, buffer);
                }
            }
            else
            {
                intersectedOutputs.remove(o);
                compositor()->imp()->graphicBackend->outputSetCursorTexture(o, nullptr);
            }

            if (cursor()->hasHardwareSupport(o))
            {
                LPointF p = newPosS - LPointF(o->pos());

                if (o->transform() == LFramebuffer::Flipped)
                    p.setX(o->rect().w() - p.x() - size.w());
                else if (o->transform() == LFramebuffer::Rotated270)
                {
                    Float32 tmp = p.x();
                    p.setX(o->rect().h() - p.y() - size.h());
                    p.setY(tmp);
                }
                else if (o->transform() == LFramebuffer::Rotated180)
                {
                    p.setX(o->rect().w() - p.x() - size.w());
                    p.setY(o->rect().h() - p.y() - size.h());
                }
                else if (o->transform() == LFramebuffer::Rotated90)
                {
                    Float32 tmp = p.x();
                    p.setX(p.y());
                    p.setY(o->rect().w() - tmp - size.h());
                }
                else if (o->transform() == LFramebuffer::Flipped270)
                {
                    Float32 tmp = p.x();
                    p.setX(o->rect().h() - p.y() - size.h());
                    p.setY(o->rect().w() - tmp - size.w());
                }
                else if (o->transform() == LFramebuffer::Flipped180)
                    p.setY(o->rect().h() - p.y() - size.y());
                else if (o->transform() == LFramebuffer::Flipped90)
                {
                    Float32 tmp = p.x();
                    p.setX(p.y());
                    p.setY(tmp);
                }

                compositor()->imp()->graphicBackend->outputSetCursorPosition(o, o->scale() * p / ( o->scale() / o->fractionalScale()) );
            }
        }

        textureChanged = false;
        posChanged = false;
    }
};

inline static void texture2Buffer(LCursor *cursor, const LSizeF &size, LFramebuffer::Transform transform)
{
    LPainter *painter = cursor->compositor()->imp()->painter;
    glBindFramebuffer(GL_FRAMEBUFFER, cursor->imp()->glFramebuffer);
    LRect src;

    if (transform == LFramebuffer::Normal ||
        transform == LFramebuffer::Flipped ||
        transform == LFramebuffer::Flipped180 ||
        transform == LFramebuffer::Rotated180)
        src = LRect(0, 0, cursor->texture()->sizeB().w(), -cursor->texture()->sizeB().h());
    else if (transform == LFramebuffer::Rotated90 ||
             transform == LFramebuffer::Rotated270 ||
             transform == LFramebuffer::Flipped90 ||
             transform == LFramebuffer::Flipped270)
        src = LRect(0, 0, -cursor->texture()->sizeB().w(), cursor->texture()->sizeB().h());

    painter->imp()->scaleCursor(
        cursor->texture(),
        src,
        LRect(0, size),
        transform);

    if (painter->imp()->openGLExtensions.EXT_read_format_bgra)
        glReadPixels(0, 0, 64, 64, GL_BGRA_EXT , GL_UNSIGNED_BYTE, cursor->imp()->buffer);
    else
    {
        glReadPixels(0, 0, 64, 64, GL_RGBA , GL_UNSIGNED_BYTE, cursor->imp()->buffer);

        UInt8 tmp;

        // Convert to RGBA8888
        for (Int32 i = 0; i < 64*64*4; i+=4)
        {
            tmp = cursor->imp()->buffer[i];
            cursor->imp()->buffer[i] = cursor->imp()->buffer[i+2];
            cursor->imp()->buffer[i+2] = tmp;
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

#endif // LCURSORPRIVATE_H
