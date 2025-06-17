#ifndef LCURSORPRIVATE_H
#define LCURSORPRIVATE_H

#include <CZ/Louvre/Private/LCompositorPrivate.h>
#include <CZ/Louvre/Private/LPainterPrivate.h>
#include <CZ/Louvre/Private/LOutputPrivate.h>
#include <CZ/Louvre/LFramebufferWrapper.h>
#include <CZ/Louvre/LClientCursor.h>
#include <CZ/Louvre/LCursor.h>
#include <CZ/Louvre/LUtils.h>

using namespace Louvre;

void texture2Buffer(LCursor *cursor, const SkSize &size, CZTransform transform) noexcept;

LPRIVATE_CLASS_NO_COPY(LCursor)
    LCursorPrivate();
    SkIRect rect { 0, 0, 0, 0 };
    SkPoint hotspotB { 0.f, 0.f };
    SkSize size { 0.f, 0.f };
    bool isVisible                                      = true;
    bool textureChanged                                 = false;
    bool posChanged                                     = false;
    bool hasFb                                          = true;
    UInt32 lastTextureSerial                            = 0;
    CZWeak<const LClientCursor> clientCursor;
    std::vector<LOutput*>intersectedOutputs;   
    LTexture *texture                                   = nullptr;
    SkPoint defaultHotspotB { 0.f, 0.f };
    LTexture *defaultTexture                            = nullptr;
    LTexture louvreTexture { true };
    GLuint glFramebuffer, glRenderbuffer;
    LFramebufferWrapper fb { 0, SkISize(64, 64) };
    UInt8 buffer[64*64*4];

    void setOutput(LOutput *out) noexcept
    {
        bool up { false };

        if (!cursor()->m_output)
            up = true;

        cursor()->m_output = out;

        if (up)
        {
            textureChanged = true;
            update();
        }
    }

    void update() noexcept
    {
        if (!cursor()->output())
            return;

        posChanged = true;
    }

    // Called once per main loop iteration
    void textureUpdate() noexcept;
};

#endif // LCURSORPRIVATE_H
