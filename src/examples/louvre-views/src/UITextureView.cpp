#include <LPainter.h>
#include <LFramebuffer.h>
#include "Global.h"
#include "UITextureView.h"

UITextureView::UITextureView(UInt32 textureIndex, LView *parent) :
    Louvre::LTextureView(nullptr, parent),
    textureIndex(textureIndex)
{
    G::setTexViewConf(this, textureIndex);
}

void UITextureView::setTextureIndex(UInt32 textureIndex)
{
    G::setTexViewConf(this, textureIndex);
    this->textureIndex = textureIndex;
}

/*
void UITextureView::paintEvent(const PaintEventParams &params)
{
    if (!mapped())
        return;

    Float32 scale;
    LTexture *texture;
    LRectF srcRect;

    if (params.painter->boundFramebuffer()->scale() == 1.f)
    {
        scale = 1.f;
        texture = G::textures()->UIConf[0][textureIndex].texture;
        srcRect = G::textures()->UIConf[0][textureIndex].customSrcRect;
    }
    else if (params.painter->boundFramebuffer()->scale() == 1.25f)
    {
        scale = 1.25f;
        texture = G::textures()->UIConf[1][textureIndex].texture;
        srcRect = G::textures()->UIConf[1][textureIndex].customSrcRect;
    }
    else if (params.painter->boundFramebuffer()->scale() == 1.5f)
    {
        scale = 1.5f;
        texture = G::textures()->UIConf[2][textureIndex].texture;
        srcRect = G::textures()->UIConf[2][textureIndex].customSrcRect;
    }
    else if (params.painter->boundFramebuffer()->scale() == 1.75f)
    {
        scale = 1.75f;
        texture = G::textures()->UIConf[3][textureIndex].texture;
        srcRect = G::textures()->UIConf[3][textureIndex].customSrcRect;
    }
    else
    {
        scale = 2.f;
        texture = G::textures()->UIConf[4][textureIndex].texture;
        srcRect = G::textures()->UIConf[4][textureIndex].customSrcRect;
    }

    params.painter->bindTextureMode({
        .texture = texture,
        .pos = pos(),
        .srcRect = srcRect,
        .dstSize = size(),
        .srcTransform = transform(),
        .srcScale = scale,
    });

    params.painter->enableCustomTextureColor(customColorEnabled());
    params.painter->setColor(customColor());
    params.painter->drawRegion(*params.region);
}
*/
