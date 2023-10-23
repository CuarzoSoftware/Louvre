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

void Keyboard::keyEvent(UInt32 keyCode, KeyState keyState)
{
    Output *output = (Output*)cursor()->output();

    if (keyState == Pressed)
    {
        if (isKeyCodePressed(KEY_LEFTCTRL))
        {
            // Switch workspace
            if (output && output->currentWorkspace && isKeyCodePressed(KEY_LEFTALT))
            {
                if (keyCode == KEY_RIGHT && std::next(output->currentWorkspace->outputLink) != output->workspaces.end())
                {
                    output->setWorkspace(*std::next(output->currentWorkspace->outputLink), 512);
                    return;
                }
                else if (keyCode == KEY_LEFT && output->currentWorkspace != output->workspaces.front())
                {
                    output->setWorkspace(*std::prev(output->currentWorkspace->outputLink), 512);
                    return;
                }
            }

            if (isKeyCodePressed(KEY_LEFTSHIFT))
            {
                // Change output mode
                if (keyCode == KEY_M)
                {
                    const LOutputMode *mode = cursor()->output()->currentMode();

                    if (mode == cursor()->output()->modes().back())
                    {
                        cursor()->output()->setMode(cursor()->output()->modes().front());
                    }
                    else
                    {
                        bool found = false;

                        for (LOutputMode *om : cursor()->output()->modes())
                        {
                            if (found)
                            {
                                mode = om;
                                break;
                            }

                            if (om == mode)
                                found = true;
                        }

                        cursor()->output()->setMode(mode);
                    }
                }

                // Screenshot
                else if (keyCode == KEY_3)
                {
                    if (cursor()->output()->bufferTexture(0))
                    {
                        const char *user = getenv("HOME");

                        if (!user)
                            return;

                        char path[128];
                        char timeString[32];

                        time_t currentTime;
                        struct tm *timeInfo;

                        time(&currentTime);
                        timeInfo = localtime(&currentTime);
                        strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", timeInfo);

                        sprintf(path, "%s/Desktop/LouvreViews_Screenshoot_%s.png", user, timeString);

                        cursor()->output()->bufferTexture(0)->save(path);
                    }
                }
                /*
                // Pause graphic backend
                else if (keyCode == KEY_P)
                {
                    compositor()->imp()->unlock();
                    compositor()->imp()->graphicBackend->pause();
                    compositor()->imp()->lock();
                }
                // Resume graphic backend
                else if (keyCode == KEY_R)
                {
                    compositor()->imp()->unlock();
                    compositor()->imp()->graphicBackend->resume();
                    compositor()->imp()->lock();
                }*/
            }
        }
    }

    G::scene()->handleKeyEvent(keyCode, keyState);
}

void Keyboard::focusChanged()
{
    /* Here we use the current keyboard focus client to set the topbar app name */

    LTexture *topbarTitleTexture = nullptr;

    if (focusSurface())
    {
        Client *client = (Client*)focusSurface()->client();

        if (client->app && client->app->nameTexture)
            topbarTitleTexture = client->app->nameTexture;
    }
    else
    {
        topbarTitleTexture = G::toplevelTextures().defaultTopbarAppName;
    }

    for (Output *output : G::outputs())
    {
        if (output->topbar)
        {
            output->topbar->appName->setTexture(topbarTitleTexture);
            output->topbar->update();
        }
    }
}
