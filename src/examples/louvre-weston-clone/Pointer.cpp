#include "Pointer.h"
#include <LTime.h>
#include <LCursor.h>
#include <cstdio>
#include <math.h>

Pointer::Pointer(Params *params) : LPointer(params)
{
    timespec ts = LTime::us();
    lastEventMs = float(ts.tv_nsec)/100000.f + float(ts.tv_sec)*10000.f;
}

void Pointer::pointerMoveEvent(float dx, float dy)
{
    timespec ts = LTime::us();
    float ct = float(ts.tv_nsec)/100000.f + float(ts.tv_sec)*10000.f;
    float dt =  ct - lastEventMs;
    lastEventMs = ct;
    float speed = 1.4f;

    if(dt >= 1.f)
        speed += 0.5f*sqrt(dx*dx + dy*dy)/dt;

    pointerPosChangeEvent(
                cursor()->posC().x() + dx*speed,
                cursor()->posC().y() + dy*speed);
}
