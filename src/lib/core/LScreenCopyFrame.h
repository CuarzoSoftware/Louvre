#ifndef LSCREENCOPYFRAME_H
#define LSCREENCOPYFRAME_H

#include <LWeak.h>

class Louvre::LScreenCopyFrame
{
public:
    bool compositeCursor() const noexcept;
    LClient *client() const noexcept;
    const LRect &rect() const noexcept;
    void copy(const LRegion *damage = nullptr) const noexcept;
    void reject() const noexcept;
private:
    friend class LOutput;
    friend class Protocols::ScreenCopy::RScreenCopyFrame;
    LScreenCopyFrame(Protocols::ScreenCopy::RScreenCopyFrame &frame) noexcept;
    ~LScreenCopyFrame() noexcept;
    Protocols::ScreenCopy::RScreenCopyFrame &m_frame;
    LWeak<LOutput> m_output;
};

#endif // LSCREENCOPYFRAME_H
