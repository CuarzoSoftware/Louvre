#include <LOpenGL.h>
#include <LTexture.h>
#include <LCursor.h>
#include <LLauncher.h>
#include <LUtils.h>
#include <LLog.h>

#include <unistd.h>

#include "Global.h"
#include "App.h"
#include "DockApp.h"
#include "Client.h"
#include "Surface.h"
#include "Output.h"
#include "Dock.h"
#include "Toplevel.h"
#include "Workspace.h"
#include "Compositor.h"

#include "../../common/TextRenderer.h"

static inline Float32 easeIn(Float32 t, Float32 exponent)
{
    return 1.0 - powf(1.0 - t, exponent);
}

static inline Float32 easeOut(Float32 t, Float32 exponent)
{
    return 1.0 - powf(t, exponent);
}

static inline Float32 periodicEasing(Float32 t, Float32 exponent_in, Float32 exponent_out)
{
    Float32 floo = floorf(t);
    Float32 norm = t - floo;
    Float32 floo2 = floo/2.0;

    if (floo2 - floorf(floo2) == 0.0)
        return easeIn(norm, exponent_in);
    else
        return easeOut(norm, exponent_out);
}

App::App(const char *appName, const char *appExec, const char *iconPath) :
    launchAnimation(15000,
    [this](LAnimation *anim)
    {
        Float32 offsetY = periodicEasing(anim->value() * 37.0, 2.0, 1.6);

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
    {
        if (name == "Wofi")
        {
            isWofi = true;
            G::compositor()->wofi = this;
        }
        exec = appExec;
    }

    LTexture *tmp = { nullptr };

    if (iconPath)
        tmp = LOpenGL::loadTexture(iconPath);

    if (tmp)
    {
        LTexture *hires = tmp->copy(LSize(DOCK_ITEM_HEIGHT * 4));
        texture = hires->copy(LSize(DOCK_ITEM_HEIGHT * 2));
        delete hires;
        delete tmp;
    }

    if (!texture)
        texture = G::textures()->defaultAppIcon;

    for (Output *output : G::outputs())
        new DockApp(this, &output->dock);

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
        LVectorRemoveOneUnordered(G::apps(), this);
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
        for (Surface *surf : G::surfaces())
        {
            if (surf->client() == client)
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
                        surf->toplevel()->configureState(surf->toplevel()->pendingConfiguration().state | LToplevelRole::Activated);
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
