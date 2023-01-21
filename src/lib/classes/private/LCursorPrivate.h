#ifndef LCURSORPRIVATE_H
#define LCURSORPRIVATE_H

#include <LCursor.h>

class Louvre::LCursor::LCursorPrivate
{
public:
    LCursorPrivate()                                    = default;
    ~LCursorPrivate()                                   = default;

    LCursorPrivate(const LCursorPrivate&)               = delete;
    LCursorPrivate &operator=(const LCursorPrivate&)    = delete;

    LCursor *cursor;
    LRect rectC;

    void update();
    void globalScaleChanged(Int32 oldScale, Int32 newScale);

    LTexture *texture                                   = nullptr;
    LOutput *output                                     = nullptr;
    LPointF posC;
    LPointF hotspotB;
    LSizeF sizeS;
    std::list<LOutput*>intersectedOutputs;
    LTexture *defaultTexture;

    bool isVisible                                      = true;


};

#endif // LCURSORPRIVATE_H
