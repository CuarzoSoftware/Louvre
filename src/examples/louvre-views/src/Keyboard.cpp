#include <LSurface.h>
#include <LCursor.h>
#include <LCompositor.h>
#include <private/LCompositorPrivate.h>
#include <LScene.h>
#include <unistd.h>
#include <time.h>

#include "Keyboard.h"
#include "Global.h"
#include "Output.h"
#include "Topbar.h"
#include "App.h"
#include "Client.h"
#include "Workspace.h"

Keyboard::Keyboard(Params *params) : LKeyboard(params) {}

void Keyboard::keyModifiersEvent(UInt32 depressed, UInt32 latched, UInt32 locked, UInt32 group)
{
    G::scene()->handleKeyModifiersEvent(depressed, latched, locked, group);
}

static Float32 ease = 2.3f;
void Keyboard::keyEvent(UInt32 keyCode, KeyState keyState)
{
    Output *output = (Output*)cursor()->output();

    if (keyState == Pressed)
    {
        bool LEFT_META  = isKeyCodePressed(KEY_LEFTMETA);
        bool LEFT_CTRL  = isKeyCodePressed(KEY_LEFTCTRL);
        bool LEFT_SHIFT = isKeyCodePressed(KEY_LEFTSHIFT);

        /* Turn ON / OFF V-Sync */
        if (LEFT_META && LEFT_SHIFT && keyCode == KEY_V)
        {
            output->enableVSync(!output->vSyncEnabled());
            output->topbar->update();
        }

        if (isKeyCodePressed(KEY_LEFTCTRL))
        {
            // Switch workspace
            if (output && output->currentWorkspace && isKeyCodePressed(KEY_LEFTALT))
            {
                if (keyCode == KEY_RIGHT && std::next(output->currentWorkspace->outputLink) != output->workspaces.end())
                {
                    if (!output->animatedFullscreenToplevel)
                        output->setWorkspace(*std::next(output->currentWorkspace->outputLink), 500.f, ease);
                    //ease += 0.1f;
                    return;
                }
                else if (keyCode == KEY_LEFT && output->currentWorkspace != output->workspaces.front())
                {
                    if (!output->animatedFullscreenToplevel)
                        output->setWorkspace(*std::prev(output->currentWorkspace->outputLink), 500.f, ease);
                    //ease += 0.1f;
                    return;
                }
            }

            if (isKeyCodePressed(KEY_LEFTSHIFT))
            {
                // Change output mode
                if (keyCode == KEY_M)
                {
                    const LOutputMode *mode = output->currentMode();

                    if (mode == output->modes().back())
                    {
                        output->setMode(output->modes().front());
                    }
                    else
                    {
                        bool found = false;

                        for (LOutputMode *om : output->modes())
                        {
                            if (found)
                            {
                                mode = om;
                                break;
                            }

                            if (om == mode)
                                found = true;
                        }

                        output->setMode(mode);
                    }
                }

                if (keyCode == KEY_T)
                {
                    Int32 trans = (Int32)output->transform();

                    if (trans == LFramebuffer::Flipped270)
                        output->setTransform(LFramebuffer::Normal);
                    else
                        output->setTransform((LFramebuffer::Transform)(trans+1));
                }

                // Increase fractional scaling by 0.25x
                else if (keyCode == KEY_UP)
                {
                    if (output->fractionalScale() < 3.f)
                    {
                        output->setScale(output->fractionalScale() + 0.25);
                        output->repaint();
                    }
                }

                // Decrease fractional scaling by 0.25x
                else if (keyCode == KEY_DOWN)
                {
                    if (output->fractionalScale() > 0.25f)
                    {
                        output->setScale(output->fractionalScale() - 0.25);
                        output->repaint();
                    }
                }
            }
        }
    }

    G::scene()->handleKeyEvent(keyCode, keyState);
}

void Keyboard::focusChanged()
{
    /* Here we use the current keyboard focus client to set the topbar app name */

    LTexture *topbarTitleTexture = nullptr;

    if (focus())
    {
        Client *client = (Client*)focus()->client();

        if (client->app && client->app->nameTexture)
            topbarTitleTexture = client->app->nameTexture;
    }
    else
    {
        topbarTitleTexture = G::textures()->defaultTopbarAppName;
    }

    for (Output *output : G::outputs())
    {
        if (output->topbar)
        {
            output->topbar->appName.setTexture(topbarTitleTexture);
            output->topbar->update();
        }
    }
}
