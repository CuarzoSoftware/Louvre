#include <LSurface.h>
#include <LCursor.h>
#include "Keyboard.h"
#include "Global.h"
#include <LScene.h>
#include <unistd.h>
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

void Keyboard::keyEvent(UInt32 keyCode, UInt32 keyState)
{
    Output *output = (Output*)cursor()->output();

    if (output && output->currentWorkspace && keyState == Pressed && isKeyCodePressed(KEY_LEFTALT) && isKeyCodePressed(KEY_LEFTCTRL))
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
