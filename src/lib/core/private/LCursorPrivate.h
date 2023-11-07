#ifndef LCURSORPRIVATE_H
#define LCURSORPRIVATE_H

#include <private/LCompositorPrivate.h>
#include <private/LPainterPrivate.h>
#include <LOutput.h>
#include <LCursor.h>
#include <algorithm>

using namespace Louvre;

inline static void texture2Buffer(LCursor *cursor, const LSizeF &size);

LPRIVATE_CLASS(LCursor)
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
    LTexture *texture                                   = nullptr;
    LPointF defaultHotspotB;
    LTexture *defaultTexture                            = nullptr;
    LTexture *louvreTexture                             = nullptr;
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
                    texture2Buffer(cursor(), size*o->scale());
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
                LPointF p = newPosS - LPointF(o->pos());
                compositor()->imp()->graphicBackend->setCursorPosition(o, p*o->scale());
            }
        }

        textureChanged = false;
        posChanged = false;
    }
};

inline static void texture2Buffer(LCursor *cursor, const LSizeF &size)
{
    glBindFramebuffer(GL_FRAMEBUFFER, cursor->imp()->glFramebuffer);
    cursor->compositor()->imp()->painter->imp()->scaleCursor(
        cursor->texture(),
        LRect(0, 0, cursor->texture()->sizeB().w(), -cursor->texture()->sizeB().h()),
        LRect(0, size));

    glReadPixels(0, 0, 64, 64, GL_RGBA , GL_UNSIGNED_BYTE, cursor->imp()->buffer);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

#endif // LCURSORPRIVATE_H
