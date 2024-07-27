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

    void pong(UInt32 serial) noexcept override;
    void createUnpinnedApp();

    App *app { nullptr };
    Int32 pid { -1 };
    bool destroyed { false };

    LTimer pingTimer;
    UInt32 lastPingSerial { 0 };
    UInt32 lastPongSerial { 0 };
    UInt32 unresponsiveCount { 0 };

    // Darken the surfaces
    LAnimation unresponsiveAnim;
};

#endif // CLIENT_H
