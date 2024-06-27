#include <LCompositor.h>
#include <LTexture.h>
#include <LLog.h>
#include <fontconfig/fontconfig.h>
#include <freetype/freetype.h>
#include <cstring>
#include "Clock.h"
#include "Output.h"

Clock::Clock() noexcept
{
    font.reset(TextRenderer::loadFont("Arial"));

    if (font)
    {
        minuteTimer.setCallback([this](LTimer *timer) {
            updateClockTexture();
            timer->start(millisecondsUntilNextMinute() + 1500);
        });

        minuteTimer.start(1);
        LLog::debug("[louvre-weston-clone] Created clock timer.");
    }
    else
        LLog::error("[louvre-weston-clone] Failed to load Arial font, the clock will not be displayed.");
}

void Clock::updateClockText()
{
    time_t rawtime;
    struct tm *timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(text, sizeof(text), "%a %b %d, %I:%M %p", timeinfo);
}

void Clock::updateClockTexture()
{
    if (!font)
        return;

    static constexpr Int32 fontSize { 2 * (32 - 16) };
    updateClockText();
    texture.reset(font->renderText(text, fontSize));

    for (Output *o : (std::vector<Output*>&)compositor()->outputs())
    {
        o->redrawClock = true;
        o->repaint();
    }
}

Int32 Clock::millisecondsUntilNextMinute()
{
    time_t rawtime;
    struct tm *timeinfo;
    struct timespec spec;

    clock_gettime(CLOCK_REALTIME, &spec);
    rawtime = spec.tv_sec;
    timeinfo = localtime(&rawtime);

    Int32 secondsUntilNextMinute = 60 - timeinfo->tm_sec;
    Int32 msUntilNextMinute = secondsUntilNextMinute * 1000 - spec.tv_nsec / 1000000;

    return msUntilNextMinute;
}
