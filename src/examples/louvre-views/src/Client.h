#ifndef CLIENT_H
#define CLIENT_H

#include <LClient.h>

class App;

using namespace Louvre;

class Client : public LClient
{
public:
    Client(LClient::Params *params);
    ~Client();

    void createNonPinnedApp();

    App *app = nullptr;
    Int32 pid = -1;
    bool destroyed = false;
};

#endif // CLIENT_H
