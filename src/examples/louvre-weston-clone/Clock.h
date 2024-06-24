#ifndef CLOCK_H
#define CLOCK_H

#include <LRect.h>
#include <LTimer.h>
#include <memory>
#include <freetype/freetype.h>

using namespace Louvre;

class Clock
{
public:
    Clock() noexcept;
    bool loadFont(const char *fontName);
    void updateClockText();
    void updateClockTexture();
    static Int32 millisecondsUntilNextMinute();

    std::unique_ptr<LTexture> texture;
    bool loadedFont { false };
    FT_Library ft;
    FT_Face face;
    LTimer minuteTimer;
    char text[128];
};

#endif // CLOCK_H
