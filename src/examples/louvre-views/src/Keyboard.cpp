#include <LSurface.h>
#include "Keyboard.h"
#include "Global.h"
#include <LScene.h>
#include <unistd.h>
#include "Output.h"
#include "Topbar.h"
#include "App.h"
#include "Client.h"

Keyboard::Keyboard(Params *params) : LKeyboard(params) {}

void Keyboard::keyModifiersEvent(UInt32 depressed, UInt32 latched, UInt32 locked, UInt32 group)
{
    G::scene()->handleKeyModifiersEvent(depressed, latched, locked, group);
}

void Keyboard::keyEvent(UInt32 keyCode, UInt32 keyState)
{
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
