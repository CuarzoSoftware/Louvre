#include <private/LTexturePrivate.h>
#include <private/LCompositorPrivate.h>
#include <private/LCursorPrivate.h>
#include <private/LOutputPrivate.h>

void LTexture::LTexturePrivate::deleteTexture()
{
    if (cursor())
    {
        if (texture == cursor()->imp()->defaultTexture)
            compositor()->cursor()->replaceDefaultB(nullptr, 0);
        if (texture == cursor()->texture())
            cursor()->useDefault();
    }

    serial++;

    if (texture->sourceType() == Framebuffer)
        return;

    if (texture->sourceType() == Native)
    {
        if (nativeOutput)
            nativeOutput->imp()->nativeTexturesToDestroy.push_back(nativeId);
        else
            compositor()->imp()->nativeTexturesToDestroy.push_back(nativeId);

        graphicBackendData = nullptr;
        return;
    }

    if (graphicBackendData)
    {
        compositor()->imp()->graphicBackend->textureDestroy(texture);
        graphicBackendData = nullptr;
    }
}
