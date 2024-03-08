#ifndef CLOCK_H
#define CLOCK_H

#include <LRect.h>
#include <freetype/freetype.h>

using namespace Louvre;

class Clock
{
public:
    Clock() noexcept;
    bool loadFont(const char *fontName);
    void updateClockText();
    void updateClockTexture();
    static Int32 timerCallback(void *data);
    static Int32 millisecondsUntilNextMinute();

    LTexture *texture { nullptr };
    wl_event_source *clockTimer { nullptr };
    LRect rectC;

    // Font
    char text[128];
    bool loadedFont { false };
    FT_Library ft;
    FT_Face face;
};

#endif // CLOCK_H
