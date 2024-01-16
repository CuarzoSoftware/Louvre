#include <string.h>
#include <LOpenGL.h>
#include <LTexture.h>
#include <LCursor.h>
#include <LLauncher.h>
#include <LLog.h>

#include <unistd.h>
#include <algorithm>

#include "Global.h"
#include "App.h"
#include "DockApp.h"
#include "Client.h"
#include "Surface.h"
#include "Output.h"
#include "Dock.h"
#include "Toplevel.h"
#include "TextRenderer.h"
#include "Workspace.h"
#include "Compositor.h"

static Float64 easeIn(Float64 t, Float64 exponent)
{
    return 1.0 - pow(1.0 - t, exponent);
}

static Float64 easeOut(Float64 t, Float64 exponent)
{
    return 1.0 - pow(t, exponent);
}

static Float64 periodic_easing_function(Float64 t, Float64 exponent_in, Float64 exponent_out)
{
    Float64 floo = floor(t);
    Float64 norm = t - floo;
    Float64 floo2 = floo/2.0;

    if (floo2 - floor(floo2) == 0.0)
        return easeIn(norm, exponent_in);
    else
        return easeOut(norm, exponent_out);
}

App::App(const char *appName, const char *appExec, const char *iconPath) :
    launchAnimation(15000,
    [this](LAnimation *anim)
    {
        Float32 offsetY = periodic_easing_function(anim->value() * 37.0, 2.0, 1.6);

        if (state == Running && offsetY < 0.08f)
        {
            anim->stop();
            return;
        }

        offsetY = 22.f * offsetY;
        dockAppsAnimationOffset.setY(round(offsetY));
        for (DockApp *dapp : dockApps)
            dapp->dock->update();
    },
    [this](LAnimation *)
    {
        dockAppsAnimationOffset.setY(0);
        for (DockApp *dapp : dockApps)
            dapp->dock->update();
    })
{
    if (!appName)
    {
        LLog::error("[louvre-views] Failed to create dock app. Missing name.");
        delete this;
        return;
    }

    name = appName;

    if (G::font()->semibold)
        nameTexture = G::font()->semibold->renderText(name.c_str(), 24, 512);

    if (!appExec)
    {
        pinned = false;
        state = Running;
    }
    else
        exec = appExec;

    LTexture *tmp = LOpenGL::loadTexture(iconPath);

    if (tmp)
    {
        LTexture *hires = tmp->copyB(LSize(DOCK_ITEM_HEIGHT * 4));
        texture = hires->copyB(LSize(DOCK_ITEM_HEIGHT * 2));
        delete hires;
        delete tmp;
    }

    if (!texture)
        texture = G::textures()->defaultAppIcon;

    for (Output *output : G::outputs())
        new DockApp(this, output->dock);

    G::apps().push_back(this);
}

App::~App()
{
    launchAnimation.stop();

    while (!dockApps.empty())
        delete dockApps.back();

    if (nameTexture)
        delete nameTexture;

    if (!pinned)
    {
        auto it = std::find(G::apps().begin(), G::apps().end(), this);
        if (it != G::apps().end())
            G::apps().erase(it);
    }
}

void App::dockIconClicked()
{
    if (state == Dead)
    {
        launchAnimation.start();
        pid = LLauncher::launch(exec);
        state = Launching;
    }
    else if (state == Running)
    {
        for (Surface *surf : (std::list<Surface*>&)client->surfaces())
        {
            if ((Client*)surf->client() == client)
            {
                if (surf->minimized())
                {
                    for (DockItem *it : surf->minimizedViews)
                    {
                        if (it->dock->output == cursor()->output())
                        {
                            surf->unminimize(it);
                            return;
                        }
                    }
                }
                else if (surf->toplevel())
                {
                    if (surf->toplevel()->fullscreen())
                    {
                        Toplevel *tl = (Toplevel*)surf->toplevel();

                        if (tl->fullscreenOutput && tl->fullscreenWorkspace)
                        {
                            tl->fullscreenOutput->setWorkspace(tl->fullscreenWorkspace, 600, 4.f);
                            return;
                        }
                    }
                    else
                    {
                        surf->toplevel()->configure(surf->toplevel()->pendingStates() | LToplevelRole::Activated);
                        surf->raise();

                        if (surf->getView()->parent() != &G::compositor()->surfacesLayer)
                        {
                            for (Output *o : G::outputs())
                            {
                                for (Workspace *ws : o->workspaces)
                                {
                                    if (&ws->surfaces == surf->getView()->parent())
                                    {
                                        o->setWorkspace(ws, 600, 4.f);
                                        return;
                                    }
                                }
                            }
                        }
                        return;
                    }

                    return;
                }
            }
        }
    }
}
