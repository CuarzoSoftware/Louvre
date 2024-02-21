#ifndef LTOUCHPOINTPRIVATE_H
#define LTOUCHPOINTPRIVATE_H

#include <LTouchPoint.h>
#include <LTouchDownEvent.h>
#include <LTouchMoveEvent.h>
#include <LTouchUpEvent.h>

using namespace Louvre;

LPRIVATE_CLASS(LTouchPoint)

bool isPressed = true;

LSurface *surface = nullptr;

LTouchDownEvent lastDownEvent;
LTouchMoveEvent lastMoveEvent;
LTouchUpEvent lastUpEvent;

LPointF pos;

std::list<LTouchPoint*>::iterator link;

void resetSerials();
void sendTouchDownEvent(const LTouchDownEvent &event);
void sendTouchFrameEvent();
void sendTouchCancelEvent();
};

#endif // LTOUCHPOINTPRIVATE_H
