#ifndef TESTVIEW_H
#define TESTVIEW_H

#include "Global.h"

#if LOUVRE_VIEWS_TESTING == 1

#include <LSolidColorView.h>

using namespace Louvre;

class TestView : public LSolidColorView
{
public:
    TestView(LView *parent);

    void touchDownEvent(const LTouchDownEvent &event) override;
    void touchMoveEvent(const LTouchMoveEvent &event) override;
    void touchUpEvent(const LTouchUpEvent &event) override;
    void touchFrameEvent(const LTouchFrameEvent &event) override;
    void touchCancelEvent(const LTouchCancelEvent &event) override;
};

#endif

#endif // TESTVIEW_H
