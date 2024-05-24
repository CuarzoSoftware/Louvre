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
#include "Toplevel.h"
#include "TextRenderer.h"
#include "Topbar.h"
#include "ToplevelView.h"
#include "Popup.h"
#include "SessionLockManager.h"

#include <LOpenGL.h>

void Compositor::initialized()
{
    // Set black as default background color
    scene.mainView()->setClearColor({0.f, 0.f, 0.f, 1.f});

    // Change the keyboard map to "latam"
    seat()->keyboard()->setKeymap(NULL, NULL, "latam", NULL);

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
        // Set scale 2 to HiDPI screens
        output->setTransform(LTransform::Normal);
        output->setScale(output->dpi() >= 200 ? 2.f : 1.f);
        output->setPos(LPoint(totalWidth, 0));
        totalWidth += output->size().w();
        compositor()->addOutput(output);
        output->repaint();
    }
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

    if (object->factoryObjectType() == LFactoryObject::Type::LPopupRole)
    {
        fadeOutSurface(static_cast<LPopupRole*>(object), 50);
        return;
    }
}

void Compositor::fadeOutSurface(LBaseSurfaceRole *role, UInt32 ms)
{
    if (role->surface() && role->surface()->mapped())
    {
        Surface *surf = (Surface*)role->surface();

        if (surf->fadedOut)
            return;

        surf->fadedOut = true;

        LTextureView *fadeOutView = new LTextureView(surf->renderThumbnail(), &fullscreenLayer);
        fadeOutView->setPos(surf->rolePos());
        fadeOutView->enableParentOffset(false);
        fadeOutView->setBufferScale(2);

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

                if (tl->current().state.check(LToplevelRole::Fullscreen | LToplevelRole::Maximized))
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
