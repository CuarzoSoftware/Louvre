#include <LLog.h>
#include <LCompositor.h>
#include <LOutput.h>
#include <LTimer.h>
#include <LPainter.h>
#include <LTexture.h>
#include <cstring>

/*
 * This test creates 12 (or more) textures: 6 from the main thread and 6 from each LOutput thread.
 * Tested formats: DRM_FORMAT_ARGB8888 and DRM_FORMAT_ABGR8888.
 * Textures are rendered on each output, arranged as shown below:
 *
 * Main Thread:   [ARGB Static] [ABGR Static] [ARGB Updated] [ABGR Updated] [ARGB Copy] [ABGR Copy]
 * Output Thread: [ARGB Static] [ABGR Static] [ARGB Updated] [ABGR Updated] [ARGB Copy] [ABGR Copy]
 *
 * Textures on the left are created once. Textures in the center are updated (pixel data rotates 90°) every 500 ms.
 * Textures on the right are copies of the initial ones.
 *
 * Missing textures (displayed as black spaces) could indicate unsupported formats or synchronization issues.
 */

using namespace Louvre;

#define ROWS 2
#define COLS 6

static UInt8 pixelsARGB[]
{
    0, 0, 255, 255,
    0, 255, 0, 255,
    255, 0, 0, 255,
    255, 255, 255, 255
};

static UInt8 pixelsABGR[]
{
    255, 0, 0, 255,
    0, 255, 0, 255,
    0, 0, 255, 255,
    255, 255, 255, 255
};

static void setNearestFilter(GLenum target, GLuint texture)
{
    glBindTexture(target, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

static void rotateBuffer(UInt8 *buffer)
{
    // Rotate the buffer pixel data 90° clockwise
    UInt32 copy[4];
    UInt32 *pixels = (UInt32*)buffer;
    memcpy(&copy, pixels, sizeof(copy));
    pixels[0] = copy[2]; // TL <- BL
    pixels[1] = copy[0]; // TR <- TL
    pixels[2] = copy[3]; // BL <- BR
    pixels[3] = copy[1]; // BR <- TR
}

class Compositor final : public LCompositor
{
public:

    void initialized() override
    {
        mainThreadARGB.setDataFromMainMemory(LSize(2, 2), 2 * 4, DRM_FORMAT_ARGB8888, pixelsARGB);
        mainThreadABGR.setDataFromMainMemory(LSize(2, 2), 2 * 4, DRM_FORMAT_ABGR8888, pixelsABGR);

        mainThreadARGB.updateRect(LRect(0, 0, 2, 2), 2 * 4, pixelsARGB);
        mainThreadABGR.updateRect(LRect(0, 0, 2, 2), 2 * 4, pixelsABGR);

        mainThreadARGBUpdated.setDataFromMainMemory(LSize(2, 2), 2 * 4, DRM_FORMAT_ARGB8888, pixelsARGB);
        mainThreadABGRUpdated.setDataFromMainMemory(LSize(2, 2), 2 * 4, DRM_FORMAT_ABGR8888, pixelsABGR);

        mainThreadARGBUpdated.updateRect(LRect(0, 0, 2, 2), 2 * 4, pixelsARGB);
        mainThreadABGRUpdated.updateRect(LRect(0, 0, 2, 2), 2 * 4, pixelsABGR);

        mainThreadARGBCopy = mainThreadARGB.copy();
        mainThreadABGRCopy = mainThreadABGR.copy();

        /* Use the default impl (it initializes all outputs) */
        LCompositor::initialized();

        /* Triggered from the main thread */
        timer.setCallback([this](LTimer *timer)
        {
            rotateBuffer(pixelsARGB);
            rotateBuffer(pixelsABGR);
            mainThreadARGBUpdated.updateRect(LRect(0, 0, 2, 2), 2 * 4, pixelsARGB);
            mainThreadABGRUpdated.updateRect(LRect(0, 0, 2, 2), 2 * 4, pixelsABGR);
            repaintAllOutputs();
            timer->start(500);
        });

        timer.start(500);
    }

    /* Deny access to all protocol */
    bool globalsFilter(LClient */*client*/, LGlobal */*global*/) override { return false; }

    LFactoryObject *createObjectRequest(LFactoryObject::Type objectType, const void *params) override;

    /* Used to update textures from the main thread every 500 ms */
    LTimer timer;
    LTexture mainThreadARGB;
    LTexture mainThreadABGR;
    LTexture mainThreadARGBUpdated;
    LTexture mainThreadABGRUpdated;
    LTexture *mainThreadARGBCopy;
    LTexture *mainThreadABGRCopy;
};

class Output final : public LOutput
{
public:
    using LOutput::LOutput;

    void initializeGL() override
    {
        painter()->setClearColor(0.f, 0.f, 0.f, 1.f);
        outputThreadARGB.setDataFromMainMemory(LSize(2, 2), 2 * 4, DRM_FORMAT_ARGB8888, pixelsARGB);
        outputThreadABGR.setDataFromMainMemory(LSize(2, 2), 2 * 4, DRM_FORMAT_ABGR8888, pixelsABGR);

        outputThreadARGB.updateRect(LRect(0, 0, 2, 2), 2 * 4, pixelsARGB);
        outputThreadABGR.updateRect(LRect(0, 0, 2, 2), 2 * 4, pixelsABGR);

        outputThreadARGBUpdated.setDataFromMainMemory(LSize(2, 2), 2 * 4, DRM_FORMAT_ARGB8888, pixelsARGB);
        outputThreadABGRUpdated.setDataFromMainMemory(LSize(2, 2), 2 * 4, DRM_FORMAT_ABGR8888, pixelsABGR);

        outputThreadARGBUpdated.updateRect(LRect(0, 0, 2, 2), 2 * 4, pixelsARGB);
        outputThreadABGRUpdated.updateRect(LRect(0, 0, 2, 2), 2 * 4, pixelsABGR);

        outputThreadARGBCopy = outputThreadARGB.copy();
        outputThreadABGRCopy = outputThreadABGR.copy();
    }

    void paintGL() override
    {
        LPainter &p { *painter() };
        Compositor &c { *static_cast<Compositor*>(compositor()) };
        LPainter::TextureParams params;
        LRect dstRect;
        params.srcTransform = LTransform::Normal;
        params.srcScale = 1.f;
        params.dstSize.setWidth(size().w() / COLS);
        params.dstSize.setHeight(size().h() / ROWS);
        params.srcRect = LRectF(0.f, 0.f, 2.f, 2.f);
        dstRect.setSize(params.dstSize);
        p.clearScreen();

        LTexture *textures[ROWS][COLS] { // [y][x]
            {&c.mainThreadARGB, &c.mainThreadABGR, &c.mainThreadARGBUpdated, &c.mainThreadABGRUpdated, c.mainThreadARGBCopy, c.mainThreadABGRCopy},
            {&outputThreadARGB, &outputThreadABGR, &outputThreadARGBUpdated, &outputThreadABGRUpdated, outputThreadARGBCopy, outputThreadABGRCopy},
        };

        outputThreadARGBUpdated.updateRect(LRect(0, 0, 2, 2), 2 * 4, pixelsARGB);
        outputThreadABGRUpdated.updateRect(LRect(0, 0, 2, 2), 2 * 4, pixelsABGR);

        for (Int32 x = 0; x < COLS; x++)
        {
            for (Int32 y = 0; y < ROWS; y++)
            {
                if (!textures[y][x])
                {
                    LLog::error("Missing texture [x:%d, y:%d].", x, y);
                    continue;
                }

                dstRect.setPos(pos() + LPoint(dstRect.w() * x, dstRect.h() * y));
                params.pos = dstRect.pos();
                params.texture = textures[y][x];
                p.bindTextureMode(params);
                setNearestFilter(params.texture->target(), params.texture->id(this));
                p.drawRect(dstRect);
            }
        }
    }

    void uninitializeGL() override
    {
        if (outputThreadARGBCopy)
            delete outputThreadARGBCopy;

        if (outputThreadABGRCopy)
            delete outputThreadABGRCopy;
    }

    LTexture outputThreadARGB;
    LTexture outputThreadABGR;
    LTexture outputThreadARGBUpdated;
    LTexture outputThreadABGRUpdated;
    LTexture *outputThreadARGBCopy;
    LTexture *outputThreadABGRCopy;
};


LFactoryObject *Compositor::createObjectRequest(LFactoryObject::Type objectType, const void *params)
{
    if (objectType == LFactoryObject::Type::LOutput)
        return new Output(params);

    return nullptr;
}

int main()
{
    Compositor compositor;
    compositor.start();

    while (compositor.state() != LCompositor::Uninitialized)
        compositor.processLoop(-1);

    return 0;
}
