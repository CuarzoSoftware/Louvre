#ifndef LCURSORPRIVATE_H
#define LCURSORPRIVATE_H

#include <LCursor.h>

LPRIVATE_CLASS(LCursor)
    LRect rect;

    void update();

    // Called once per main loop iteration
    void textureUpdate();

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
    LTexture *defaultTexture                            = nullptr;
    GLuint glFramebuffer, glRenderbuffer;
    UChar8 buffer[64*64*4];
};

#endif // LCURSORPRIVATE_H
