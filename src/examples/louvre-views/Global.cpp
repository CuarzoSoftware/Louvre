#include <Output.h>
#include <LLog.h>
#include <Global.h>
#include <Compositor.h>
#include <string.h>
#include <LXCursor.h>

static G::BorderRadiusTextures borderRadiusTextures;
static G::Cursors xCursors;

Compositor *G::compositor()
{
    return (Compositor*)LCompositor::compositor();
}

LScene *G::scene()
{
    return compositor()->scene;
}

std::list<Output *> &G::outputs()
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
        output->repaint();
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
            else if (rad > circleRadius &&  rad <= circleRadius + 10)
                circleBuffer[x*4 + y*circleRadius*4 + 3] = 200;
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

void G::loadCursors()
{
    xCursors.handCursor = LXCursor::loadXCursorB("hand2");
}

G::Cursors &G::cursors()
{
    return xCursors;
}
