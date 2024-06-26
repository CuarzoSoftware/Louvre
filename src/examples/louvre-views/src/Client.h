#ifndef CLIENT_H
#define CLIENT_H

#include <LClient.h>
#include <LTimer.h>
#include <LAnimation.h>

class App;

using namespace Louvre;

class Client final : public LClient
{
public:
    Client(const void *params);
    ~Client();

    // Used to check if the client is alive
    void pong(UInt32 serial) noexcept override;

    void createNonPinnedApp();

    App *app { nullptr };
    Int32 pid { -1 };
    bool destroyed { false };

    LTimer pingTimer;
    UInt32 lastPing { 0 };
    UInt32 lastPong { 0 };

    UInt32 unresponsiveCount { 0 };
    LAnimation unresponsiveAnim;
};

#endif // CLIENT_H
