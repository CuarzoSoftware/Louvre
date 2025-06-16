#include <CZ/Louvre/Private/LCursorPrivate.h>

LCursor::LCursorPrivate::LCursorPrivate() : defaultTexture() {}

void LCursor::LCursorPrivate::textureUpdate() noexcept
{
    if (!cursor()->output())
        return;

    if (!textureChanged && !posChanged)
        return;

    const SkSize sizeBckp { size };

    if (compositor()->graphicBackendId() == LGraphicBackendWayland)
        size = SkSize(64.f / cursor()->output()->scale());

    const SkPoint newHotspotS {
        (hotspotB.x() * size.width())/SkScalar(texture->sizeB().width()),
        (hotspotB.y() * size.height())/SkScalar(texture->sizeB().height())
    };

    const SkPoint newPosS { cursor()->pos() - newHotspotS };
    rect.setXYWH(newPosS.x(), newPosS.y(), size.width(), size.height());

    for (LOutput *o : compositor()->outputs())
    {
        if (isVisible && SkIRect::Intersects(o->rect(), rect))
        {
            const bool found { std::find(intersectedOutputs.begin(), intersectedOutputs.end(), o) != intersectedOutputs.end() };

            if (!found)
                intersectedOutputs.push_back(o);

            if (cursor()->hasHardwareSupport(o) && (textureChanged || !found))
            {
                if (cursor()->enabled(o) && cursor()->hwCompositingEnabled(o))
                {
                    texture2Buffer(cursor(),
                                   SkSize(size.width() * o->fractionalScale(),
                                          size.height() * o->fractionalScale()),
                                   o->transform());
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
            SkPoint p { newPosS - SkPoint::Make(o->pos().x(), o->pos().y()) };

            if (o->transform() == CZTransform::Flipped)
                p.fX = o->rect().width() - p.x() - size.width();
            else if (o->transform() == CZTransform::Rotated270)
            {
                const Float32 tmp { p.x() };
                p.fX = o->rect().height() - p.y() - size.height();
                p.fY = tmp;
            }
            else if (o->transform() == CZTransform::Rotated180)
            {
                p.fX = o->rect().width() - p.x() - size.width();
                p.fY = o->rect().height() - p.y() - size.height();
            }
            else if (o->transform() == CZTransform::Rotated90)
            {
                const Float32 tmp { p.x() };
                p.fX = p.y();
                p.fY = o->rect().width() - tmp - size.height();
            }
            else if (o->transform() == CZTransform::Flipped270)
            {
                const Float32 tmp { p.x() };
                p.fX = o->rect().height() - p.y() - size.height();
                p.fY = o->rect().width() - tmp - size.width();
            }
            else if (o->transform() == CZTransform::Flipped180)
                p.fY = o->rect().height() - p.y() - size.height();
            else if (o->transform() == CZTransform::Flipped90)
            {
                const Float32 tmp { p.x() };
                p.fX = p.y();
                p.fY = tmp;
            }

            compositor()->imp()->graphicBackend->outputSetCursorPosition(
                o,
                SkIPoint::Make(
                    p.x() * o->fractionalScale(),
                    p.y() * o->fractionalScale()));
        }
    }

    size = sizeBckp;

    textureChanged = false;
    posChanged = false;
}

void texture2Buffer(LCursor *cursor, const SkSize &size, CZTransform transform) noexcept
{
    eglMakeCurrent(compositor()->eglDisplay(), EGL_NO_SURFACE, EGL_NO_SURFACE, compositor()->eglContext());
    LPainter *painter { compositor()->imp()->painter };
    painter->bindProgram();
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
        .pos = SkIPoint(0, 0),
        .srcRect = SkRect::MakeWH(cursor->texture()->sizeB().width(), cursor->texture()->sizeB().height()),
        .dstSize = SkISize(size.width(), size.height()),
        .srcTransform = CZ::RequiredTransform(transform, CZTransform::Normal),
        .srcScale = 1.f,
    });
    glDisable(GL_BLEND);
    painter->drawRect(SkIRect::MakeWH(size.width(), size.height()));
    glEnable(GL_BLEND);

    glFinish(); // TODO: Replace with sync

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
