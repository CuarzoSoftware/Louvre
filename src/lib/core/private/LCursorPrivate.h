#ifndef LCURSORPRIVATE_H
#define LCURSORPRIVATE_H

#include <LCursor.h>

LPRIVATE_CLASS(LCursor)
    LRect rectC;

    void update();

    // Called once per main loop iteration
    void textureUpdate();

    void globalScaleChanged(Int32 oldScale, Int32 newScale);

    LPointF posC;
    LPointF hotspotB;
    LSizeF sizeS;
    LOutput *output                                     = nullptr;
    std::list<LOutput*>intersectedOutputs;
    bool isVisible                                      = true;


    UInt32 lastTextureSerial                            = 0;
    bool textureChanged                                 = false;
    LTexture *texture                                   = nullptr;
    LTexture *defaultTexture                            = nullptr;
    GLuint glFramebuffer, glRenderbuffer;
    UChar8 buffer[64*64*4];
};

#endif // LCURSORPRIVATE_H
