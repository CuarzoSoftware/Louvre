#include <private/LCursorPrivate.h>

LCursor::LCursorPrivate::LCursorPrivate() : defaultTexture() {}

void LCursor::LCursorPrivate::textureUpdate() noexcept
{
    if (!cursor()->output())
        return;

    if (!textureChanged && !posChanged)
        return;

    const LSizeF sizeBckp { size };

    if (compositor()->graphicBackendId() == LGraphicBackendWayland)
        size = LSizeF(64.f / cursor()->output()->scale());

    const LPointF newHotspotS { (hotspotB*size)/LSizeF(texture->sizeB()) };
    const LPointF newPosS { cursor()->pos() - newHotspotS };

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

    size = sizeBckp;

    textureChanged = false;
    posChanged = false;
}

void texture2Buffer(LCursor *cursor, const LSizeF &size, LTransform transform) noexcept
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

    glFinish();

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
