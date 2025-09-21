#include "TestView.h"

#if LOUVRE_VIEWS_TESTING == 1

#include <LCompositor.h>
#include <LTouchDownEvent.h>
#include <LTouchMoveEvent.h>
#include <LTouchUpEvent.h>

TestView::TestView(LView *parent)
    : LSolidColorView(0.5f, 0.5f, 0.5f, 1.f, parent) {
  setSize(400, 400);
  setPos(2000, 200);

  enableTouchEvents(true);
}

void TestView::touchDownEvent(const LTouchDownEvent &event) {
  setColor({0, 1, 0});
  for (LView *v : children()) {
    if (v->userData() == (UIntPtr)event.id()) {
      static_cast<LSolidColorView *>(v)->setPos(event.localPos);
      return;
    }
  }

  new LSolidColorView(0, 0, 0, 1, this);
  children().back()->setUserData(event.id());
  static_cast<LSolidColorView *>(children().back())->setPos(event.localPos);
  static_cast<LSolidColorView *>(children().back())->setSize(10, 10);
  compositor()->repaintAllOutputs();
}

void TestView::touchMoveEvent(const LTouchMoveEvent &event) {
  setColor({0, 1, 1});

  for (LView *v : children()) {
    if (v->userData() == (UIntPtr)event.id()) {
      static_cast<LSolidColorView *>(v)->setPos(event.localPos);
      return;
    }
  }
  compositor()->repaintAllOutputs();
}

void TestView::touchUpEvent(const LTouchUpEvent &event) {
  setColor({0.5, 0.5, 0.5});

  compositor()->repaintAllOutputs();

  for (LView *v : children()) {
    if (v->userData() == (UIntPtr)event.id()) {
      delete v;
      return;
    }
  }
}

void TestView::touchFrameEvent(const LTouchFrameEvent &event) {
  compositor()->repaintAllOutputs();
}

void TestView::touchCancelEvent(const LTouchCancelEvent &event) {
  setColor({0.0, 0.0, 0.0});
}

#endif
