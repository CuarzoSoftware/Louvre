#include <string.h>
#include <LOpenGL.h>
#include <LTexture.h>
#include <LCursor.h>
#include <unistd.h>
#include "Global.h"
#include "App.h"
#include "DockApp.h"
#include "Surface.h"
#include "Output.h"
#include "Dock.h"
#include "Toplevel.h"
#include "TextRenderer.h"

App::App(const char *name, const char *exec, const char *iconPath)
{
    if (name)
    {
        strcpy(this->name, name);

        if (G::font()->semibold)
            nameTexture = G::font()->semibold->renderText(name, 24, 512);
    }
    else
    {
        delete this;
        return;
    }

    if (exec)
        strcpy(this->exec, exec);
    else
    {
        pinned = false;
        state = Running;
    }

    LTexture *tmp = LOpenGL::loadTexture(iconPath);

    if (tmp)
    {
        LTexture *hires = tmp->copyB(LSize(DOCK_ITEM_HEIGHT * 4));
        texture = hires->copyB(LSize(DOCK_ITEM_HEIGHT * 2));
        delete hires;
        delete tmp;
    }

    if (!texture)
        texture = G::dockTextures().defaultApp;

    for (Output *output : G::outputs())
        new DockApp(this, output->dock);

    G::apps().push_back(this);
}

App::~App()
{
    if (launchAnimation)
        launchAnimation->stop();

    while (!dockApps.empty())
        delete dockApps.back();

    if (nameTexture)
        delete nameTexture;
}

static Float32 easeIn(Float32 t, Float32 exponent)
{
    return pow(t, exponent);
}

static Float32 easeOut(Float32 t, Float32 exponent)
{
    return 1.0 - pow(1.0 - t, exponent);
}

static Float32 periodic_easing_function(Float32 t, Float32 period, Float32 exponent_in, Float32 exponent_out)
{
    Float32 normalized_t = fmod(t, period) / period;

    if (fmod(truncf(t), 2.f) == 0.f)
        return easeIn(sinf(normalized_t * M_PI), exponent_in);
    else
        return easeOut(sinf(normalized_t * M_PI), exponent_out);
}

void App::clicked()
{
    if (state == Dead)
    {
        launchAnimation = LAnimation::create(15000,
        [this](LAnimation *anim)
        {
            Float32 offsetY = periodic_easing_function(anim->value() * 38.f, 2.f, 2.f, 2.f);

            if (state == Running && offsetY < 0.08f)
            {
                anim->stop();
                return;
            }

            offsetY = 18.f * offsetY;
            dockAppsAnimationOffset.setY(roundf(offsetY));
            for (DockApp *dapp : dockApps)
                dapp->dock->update();
        },
        [this](LAnimation *)
        {
            dockAppsAnimationOffset.setY(0);
            for (DockApp *dapp : dockApps)
                dapp->dock->update();
            launchAnimation = nullptr;
        });

        launchAnimation->start(true);

        pid = fork();

        if (pid == 0)
        {
            system(exec);
            abort();
        }

        state = Launching;
    }
    else if (state == Running)
    {
        std::list<Surface*> surfaces = (std::list<Surface*>&)compositor()->surfaces();

        for (Surface *surf : surfaces)
        {
            if ((Client*)surf->client() == client)
            {
                if (surf->minimized())
                {
                    for (DockItem *it : surf->minimizedViews)
                    {
                        if (it->dock->output == cursor()->output())
                        {
                            //if (it->dock->output->fullscreenToplevel && it->dock->output->fullscreenToplevel->surface() != surf)
                                //it->dock->output->fullscreenToplevel->unsetFullscreenRequest();

                            surf->unminimize(it);
                            return;
                        }
                    }
                    return;
                }
                else if (surf->toplevel())
                {
                    Output *o = (Output*)cursor()->output();
                    //if (o->fullscreenToplevel && o->fullscreenToplevel->surface() != surf)
                        //o->fullscreenToplevel->unsetFullscreenRequest();

                    surf->toplevel()->configure(surf->toplevel()->states() | LToplevelRole::Activated);
                    surf->raise();
                    return;
                }
            }
        }
    }
}
