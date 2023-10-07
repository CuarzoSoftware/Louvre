#ifndef LCURSORPRIVATE_H
#define LCURSORPRIVATE_H

#include <LCursor.h>

using namespace Louvre;

LPRIVATE_CLASS(LCursor)
    LRect rect;

    void update();

    // Called once per main loop iteration
    void textureUpdate();

    void setOutput(LOutput *output);

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
};

#endif // LCURSORPRIVATE_H
