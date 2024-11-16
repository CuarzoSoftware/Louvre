#ifndef LCURSORPRIVATE_H
#define LCURSORPRIVATE_H

#include <private/LCompositorPrivate.h>
#include <private/LPainterPrivate.h>
#include <private/LOutputPrivate.h>
#include <LFramebufferWrapper.h>
#include <LClientCursor.h>
#include <LCursor.h>
#include <LUtils.h>

using namespace Louvre;

void texture2Buffer(LCursor *cursor, const LSizeF &size, LTransform transform) noexcept;

LPRIVATE_CLASS_NO_COPY(LCursor)
    LCursorPrivate();
    LRect rect;
    LPointF hotspotB;
    LSizeF size;
    bool isVisible                                      = true;
    bool textureChanged                                 = false;
    bool posChanged                                     = false;
    bool hasFb                                          = true;
    UInt32 lastTextureSerial                            = 0;
    LWeak<const LClientCursor> clientCursor;
    std::vector<LOutput*>intersectedOutputs;   
    LTexture *texture                                   = nullptr;
    LPointF defaultHotspotB;
    LTexture *defaultTexture                            = nullptr;
    LTexture louvreTexture { true };
    LRenderBuffer fb { LSize(64, 64) };
    UChar8 buffer[64*64*4];

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
