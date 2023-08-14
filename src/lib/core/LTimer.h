#ifndef LTIMER_H
#define LTIMER_H

#include <LObject.h>
#include <functional>

using namespace Louvre;

/* The LTimer class ...*/
class Louvre::LTimer : public LObject
{
public:
    /*!
     * On Timeout allback function type.
     */
    using Callback = std::function<void(LTimer*)>;

    LTimer(const Callback &onTimeout);
    static void oneShot(UInt32 intervalMs, const Callback &onTimeout);

    LTimer(const LTimer&) = delete;
    LTimer& operator= (const LTimer&) = delete;

    /*!
     * Destructor of the LTimer class.
     */
    ~LTimer();

    void setCallback(const Callback &onTimeout);
    UInt32 interval() const;
    bool running() const;
    void cancel();
    void start(UInt32 interval);

LPRIVATE_IMP(LTimer)
};

#endif // LTIMER_H
