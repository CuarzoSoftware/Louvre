#include <Output.h>
#include <LLog.h>
#include <Global.h>
#include <Compositor.h>
#include <string.h>

static struct G::BorderRadiusTextures borderRadiusTextures;

Compositor *G::compositor()
{
    return (Compositor*)LCompositor::compositor();
}

std::list<Output *> G::outputs()
{
    return (std::list<Output*>&)compositor()->outputs();
}

void G::arrangeOutputs()
{

    Int32 totalWidth = 0;

    for (Output *output : G::outputs())
    {
        output->setScale(output->dpi() >= 120 ? 2 : 1);
        output->setPos(LPoint(totalWidth, 0));
        totalWidth += output->size().w();
    }
}

void G::createBorderRadiusTextures()
{
    Int32 circleRadius = 256;
    UChar8 circleBuffer[circleRadius*circleRadius*4];
    memset(circleBuffer, 0, sizeof(circleBuffer));

    for (Int32 x = 0; x < circleRadius; x++)
        for (Int32 y = 0; y < circleRadius; y++)
        {
            Float32 rad = sqrtf(x*x + y*y);

            if (rad <= circleRadius)
                circleBuffer[x*4 + y*circleRadius*4 + 3] = 255;
            else if (rad > circleRadius &&  rad <= circleRadius + 8)
                circleBuffer[x*4 + y*circleRadius*4 + 3] = 150;
            else
                circleBuffer[x*4 + y*circleRadius*4 + 3] = 0;
        }


    LTexture *circleTexture = new LTexture();
    circleTexture->setDataB(LSize(circleRadius, circleRadius), circleRadius*4, DRM_FORMAT_ARGB8888, circleBuffer);

    borderRadiusTextures.TL = circleTexture->copyB(32, LRect(0, 0, -circleRadius, -circleRadius));
    borderRadiusTextures.TR = circleTexture->copyB(32, LRect(0, 0,  circleRadius, -circleRadius));
    borderRadiusTextures.BR = circleTexture->copyB(32, LRect(0, 0,  circleRadius,  circleRadius));
    borderRadiusTextures.BL = circleTexture->copyB(32, LRect(0, 0, -circleRadius,  circleRadius));

    delete circleTexture;
}

G::BorderRadiusTextures *G::borderRadius()
{
    return &borderRadiusTextures;
}
