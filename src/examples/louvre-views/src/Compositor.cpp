#include <LLayerView.h>
#include <LAnimation.h>
#include <LTextureView.h>
#include <LTimer.h>
#include <LCursor.h>
#include <LLog.h>
#include <signal.h>

#include "Client.h"
#include "Global.h"
#include "Compositor.h"
#include "Output.h"
#include "Surface.h"
#include "Seat.h"
#include "Pointer.h"
#include "Keyboard.h"
#include "Touch.h"
#include "Toplevel.h"
#include "Topbar.h"
#include "ToplevelView.h"
#include "Popup.h"
#include "SessionLockManager.h"
#include "LayerRole.h"

#include "../../common/TextRenderer.h"

#include <LOpenGL.h>

#if LOUVRE_VIEWS_TESTING == 1
#include "TestView.h"
#endif

#include <LOutputMode.h>
void Compositor::initialized()
{
    // Set black as default background color
    scene.mainView()->setClearColor({0.f, 0.f, 0.f, 1.f});
    rootView.enableParentOffset(false);

    G::loadCursors();
    G::loadTextures();
    G::loadToplevelRegions();
    G::loadFonts();
    G::createTooltip();
    G::loadApps();

    clockMinuteTimer.setCallback([](LTimer *timer)
    {
        if (G::font()->regular)
        {
            char text[64];
            time_t rawtime;
            struct tm *timeinfo;
            time(&rawtime);
            timeinfo = localtime(&rawtime);
            strftime(text, sizeof(text), "%a %b %d, %I:%M %p", timeinfo);

            LTexture *newClockTexture = G::font()->regular->renderText(text, 22);

            if (newClockTexture)
            {
                for (Output *o : G::outputs())
                {
                    o->topbar.clock.setTexture(newClockTexture);
                    o->topbar.update();
                }

                if (G::compositor()->clockTexture)
                    delete G::compositor()->clockTexture;

                G::compositor()->clockTexture = newClockTexture;
            }
        }

        timer->start(millisecondsUntilNextMinute() + 1500);
    });

    // Start the timer right on to setup the clock texture
    clockMinuteTimer.start(1);
    oversamplingLabelTexture = G::font()->semibold->renderText("OVERSAMPLING", 22);
    vSyncLabelTexture = G::font()->semibold->renderText("V-SYNC", 22);

    Int32 totalWidth { 0 };

    // Initialize and arrange outputs (screens) left to right
    for (LOutput *output : seat()->outputs())
    {
        // Probably a VR headset, meant to be leased by clients
        if (output->isNonDesktop())
        {
            output->setLeasable(true);
            continue;
        }

        output->setTransform(LTransform::Normal);
        output->setScale(output->dpi() >= 200 ? 2.f : 1.f);
        output->setPos(LPoint(totalWidth, 0));
        totalWidth += output->size().w();
        compositor()->addOutput(output);
        output->repaint();
    }

    if (outputs().empty())
    {
        LLog::fatal("[louvre-views] Failed to initialize outputs. Try launching the compositor from a free TTY or within a Wayland compositor.");
        finish();
    }

    static_cast<Seat*>(seat())->configureInputDevices();

#if LOUVRE_VIEWS_TESTING == 1
    new TestView(&overlayLayer);
#endif
}

void Compositor::uninitialized()
{
    for (Output *o : G::outputs())
        o->workspacesAnimation.stop();

    for (Client *c : (std::vector<Client*>&)clients())
        if (c->pid != -1)
            kill(c->pid, SIGKILL);
}

LFactoryObject *Compositor::createObjectRequest(LFactoryObject::Type type, const void *params)
{
    if (type == LFactoryObject::Type::LSurface)
        return new Surface(params);

    if (type == LFactoryObject::Type::LToplevelRole)
        return new Toplevel(params);

    if (type == LFactoryObject::Type::LPopupRole)
        return new Popup(params);

    if (type == LFactoryObject::Type::LLayerRole)
        return new LayerRole(params);

    if (type == LFactoryObject::Type::LClient)
        return new Client(params);

    if (type == LFactoryObject::Type::LOutput)
        return new Output(params);

    if (type == LFactoryObject::Type::LSeat)
        return new Seat(params);

    if (type == LFactoryObject::Type::LPointer)
        return new Pointer(params);

    if (type == LFactoryObject::Type::LKeyboard)
        return new Keyboard(params);

    if (type == LFactoryObject::Type::LTouch)
        return new Touch(params);

    if (type == LFactoryObject::Type::LSessionLockManager)
        return new SessionLockManager(params);

    return nullptr;
}

void Compositor::onAnticipatedObjectDestruction(LFactoryObject *object)
{
    if (object->factoryObjectType() == LFactoryObject::Type::LClient)
    {
        static_cast<Client*>(object)->destroyed = true;
        return;
    }
}

void Compositor::fadeOutSurface(LBaseSurfaceRole *role, UInt32 ms)
{
    if (role->surface())
    {
        Surface *surf = (Surface*)role->surface();

        if (surf->fadedOut)
            return;

        surf->fadedOut = true;
        surf->view.enableAlwaysMapped(true);

        if (surf->tl() && surf->tl()->decoratedView)
        {
            surf->tl()->decoratedView->surfB.enableAlwaysMapped(true);
            surf->tl()->decoratedView->surfBL.enableAlwaysMapped(true);
            surf->tl()->decoratedView->surfBR.enableAlwaysMapped(true);
        }

        LTextureView *fadeOutView = new LTextureView(surf->renderThumbnail(), &fullscreenLayer);
        surf->view.enableAlwaysMapped(false);

        if (surf->tl() && surf->tl()->decoratedView)
        {
            surf->tl()->decoratedView->surfB.enableAlwaysMapped(false);
            surf->tl()->decoratedView->surfBL.enableAlwaysMapped(false);
            surf->tl()->decoratedView->surfBR.enableAlwaysMapped(false);
        }

        fadeOutView->enableParentOffset(false);
        fadeOutView->setBufferScale(2);

        if (surf->tl() && surf->tl()->decoratedView)
            fadeOutView->setPos(surf->tl()->decoratedView->decoTL.pos());
        else
            fadeOutView->setPos(surf->rolePos());

        if (surf->toplevel())
        {
            fadeOutView->enableScaling(true);
            LPoint middle { fadeOutView->pos() + fadeOutView->size()/2 };

            LAnimation::oneShot(ms,
            [fadeOutView, middle](LAnimation *anim)
            {
                Float64 x { 1.0 - pow(1.0 - anim->value(), 3.0) };
                fadeOutView->setScalingVector(LSizeF(1.0 - x * 0.25));
                fadeOutView->setPos(middle - fadeOutView->size()/2);
                fadeOutView->setOpacity(1.0 - x);
                G::compositor()->repaintAllOutputs();
            },
            [fadeOutView](LAnimation *)
            {
                fadeOutView->repaint();
                delete fadeOutView->texture();
                delete fadeOutView;
                G::compositor()->repaintAllOutputs();
            });
        }
        else
        {
            LAnimation::oneShot(ms,
                [fadeOutView](LAnimation *anim)
                {
                    fadeOutView->setOpacity(1.f - anim->value());
                    G::compositor()->repaintAllOutputs();
                },
                [fadeOutView](LAnimation *)
                {
                    fadeOutView->repaint();
                    delete fadeOutView->texture();
                    delete fadeOutView;
                    G::compositor()->repaintAllOutputs();
                });
        }
    }
}

Int32 Compositor::millisecondsUntilNextMinute()
{
    time_t rawtime;
    struct tm *timeinfo;
    struct timespec spec;

    // Get the current time
    clock_gettime(CLOCK_REALTIME, &spec);
    rawtime = spec.tv_sec;
    timeinfo = localtime(&rawtime);

    // Calculate the number of seconds until the next minute
    int secondsUntilNextMinute = 60 - timeinfo->tm_sec;

    // Calculate the number of milliseconds until the next minute
    int msUntilNextMinute = secondsUntilNextMinute * 1000 - spec.tv_nsec / 1000000;

    return msUntilNextMinute;
}

bool Compositor::checkUpdateOutputUnplug()
{
    if (!outputUnplugHandled)
    {
        outputUnplugHandled = true;

        for (Surface *s : G::surfaces())
        {
            if (s->outputUnplugHandled)
                continue;

            if (s->toplevel())
            {
                Toplevel *tl = (Toplevel*) s->toplevel();

                if (tl->state().check(LToplevelRole::Fullscreen | LToplevelRole::Maximized))
                {
                    outputUnplugHandled = false;

                    if (tl->outputUnplugConfigureCount > 128)
                    {
                        tl->surf()->client()->destroyLater();
                        return outputUnplugHandled;
                    }
                    tl->configureState(LToplevelRole::Activated);
                    tl->surf()->client()->flush();
                    tl->surf()->requestNextFrame(false);
                    tl->outputUnplugConfigureCount++;
                }
                else
                    s->outputUnplugHandled = true;

                if (tl->decoratedView)
                    tl->decoratedView->updateGeometry();
            }
            else
            {
                s->outputUnplugHandled = true;
            }
        }

        if (outputUnplugHandled)
        {
            for (Output *o : G::outputs())
                G::scene()->mainView()->damageAll(o);
        }
    }

    return outputUnplugHandled;
}
