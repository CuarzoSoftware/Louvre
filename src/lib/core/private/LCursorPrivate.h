#ifndef LCURSORPRIVATE_H
#define LCURSORPRIVATE_H

#include <private/LCompositorPrivate.h>
#include <private/LPainterPrivate.h>
#include <private/LOutputPrivate.h>
#include <LFramebufferWrapper.h>
#include <LClientCursor.h>
#include <LCursor.h>
#include <algorithm>
#include <LUtils.h>

using namespace Louvre;

inline static void texture2Buffer(LCursor *cursor, const LSizeF &size, LTransform transform);

LPRIVATE_CLASS_NO_COPY(LCursor)
    LCursorPrivate();
    LRect rect;
    LPointF pos;
    LPointF hotspotB;
    LSizeF size;
    bool isVisible                                      = true;
    bool textureChanged                                 = false;
    bool posChanged                                     = false;
    bool hasFb                                          = true;
    UInt32 lastTextureSerial                            = 0;
    LOutput *output                                     = nullptr;
    LWeak<const LClientCursor> clientCursor;
    std::vector<LOutput*>intersectedOutputs;   
    LTexture *texture                                   = nullptr;
    LPointF defaultHotspotB;
    LTexture *defaultTexture                            = nullptr;
    LTexture louvreTexture { true };
    GLuint glFramebuffer, glRenderbuffer;
    LFramebufferWrapper fb { 0, LSize(64, 64) };
    UChar8 buffer[64*64*4];

    inline void setOutput(LOutput *out)
    {
        bool up { false };

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
        for (LOutput *output : compositor()->outputs())
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
    }

    // Called once per main loop iteration
    inline void textureUpdate()
    {
        if (!cursor()->output())
            return;

        if (!textureChanged && !posChanged)
            return;

        const LPointF newHotspotS { (hotspotB*size)/LSizeF(texture->sizeB()) };
        const LPointF newPosS { pos - newHotspotS };

        rect.setPos(newPosS);
        rect.setSize(size);

        for (LOutput *o : compositor()->outputs())
        {
            if (isVisible && o->rect().intersects(rect))
            {
                const bool found { std::find(intersectedOutputs.begin(), intersectedOutputs.end(), o) != intersectedOutputs.end() };

                if (!found)
                    intersectedOutputs.push_back(o);

                if (cursor()->hasHardwareSupport(o) && (textureChanged || !found))
                {                    
                    if (cursor()->enabled(o) && cursor()->hwCompositingEnabled(o))
                    {
                        texture2Buffer(cursor(), size * o->fractionalScale(), o->transform());
                        compositor()->imp()->graphicBackend->outputSetCursorTexture(o, buffer);
                    }
                    else
                        compositor()->imp()->graphicBackend->outputSetCursorTexture(o, nullptr);
                }
            }
            else
            {
                LVectorRemoveOneUnordered(intersectedOutputs, o);
                compositor()->imp()->graphicBackend->outputSetCursorTexture(o, nullptr);
            }

            if (cursor()->enabled(o) && cursor()->hasHardwareSupport(o))
            {
                LPointF p { newPosS - LPointF(o->pos()) };

                if (o->transform() == LTransform::Flipped)
                    p.setX(o->rect().w() - p.x() - size.w());
                else if (o->transform() == LTransform::Rotated270)
                {
                    const Float32 tmp { p.x() };
                    p.setX(o->rect().h() - p.y() - size.h());
                    p.setY(tmp);
                }
                else if (o->transform() == LTransform::Rotated180)
                {
                    p.setX(o->rect().w() - p.x() - size.w());
                    p.setY(o->rect().h() - p.y() - size.h());
                }
                else if (o->transform() == LTransform::Rotated90)
                {
                    const Float32 tmp { p.x() };
                    p.setX(p.y());
                    p.setY(o->rect().w() - tmp - size.h());
                }
                else if (o->transform() == LTransform::Flipped270)
                {
                    const Float32 tmp { p.x() };
                    p.setX(o->rect().h() - p.y() - size.h());
                    p.setY(o->rect().w() - tmp - size.w());
                }
                else if (o->transform() == LTransform::Flipped180)
                    p.setY(o->rect().h() - p.y() - size.y());
                else if (o->transform() == LTransform::Flipped90)
                {
                    const Float32 tmp { p.x() };
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

inline static void texture2Buffer(LCursor *cursor, const LSizeF &size, LTransform transform)
{
    LPainter *painter { compositor()->imp()->painter };
    glBindFramebuffer(GL_FRAMEBUFFER, cursor->imp()->glFramebuffer);
    cursor->imp()->fb.setId(cursor->imp()->glFramebuffer);
    painter->bindFramebuffer(&cursor->imp()->fb);
    painter->enableCustomTextureColor(false);
    painter->setAlpha(1.f);
    painter->setColorFactor(1.f, 1.f, 1.f, 1.f);
    painter->setClearColor(0.f, 0.f, 0.f, 0.f);
    painter->clearScreen();
    painter->bindTextureMode({
        .texture = cursor->texture(),
        .pos = LPoint(0, 0),
        .srcRect = LRect(0, 0, cursor->texture()->sizeB().w(), cursor->texture()->sizeB().h()),
        .dstSize = size,
        .srcTransform = Louvre::requiredTransform(transform, LTransform::Normal),
        .srcScale = 1.f,
    });
    glDisable(GL_BLEND);
    painter->drawRect(LRect(0, size));
    glEnable(GL_BLEND);

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
}

#endif // LCURSORPRIVATE_H
