#ifndef RSCREENCOPYFRAME_H
#define RSCREENCOPYFRAME_H

#include <LScreenshotRequest.h>
#include <LResource.h>
#include <LBitset.h>
#include <LWeak.h>
#include <LRect.h>

class Louvre::Protocols::ScreenCopy::RScreenCopyFrame final : public LResource
{
public:
    enum StateFlags : UInt8
    {
        CompositeCursor     = static_cast<UInt8>(1) << 0,
        AlreadyUsed         = static_cast<UInt8>(1) << 1,
        WaitForDamage       = static_cast<UInt8>(1) << 2,
        Accepted            = static_cast<UInt8>(1) << 3
    };

    LOutput *output()       const noexcept { return m_output.get(); }
    const LRect &rect()     const noexcept { return m_rect; };
    const LRect &rectB()    const noexcept { return m_rectB; };
    bool compositeCursor()  const noexcept { return m_stateFlags.check(CompositeCursor); };
    bool alreadyUsed()      const noexcept { return m_stateFlags.check(AlreadyUsed); };
    bool waitForDamage()    const noexcept { return m_stateFlags.check(WaitForDamage); };
    bool accepted()           const noexcept { return m_stateFlags.check(Accepted); };
    wl_resource *buffer()   const noexcept { return m_bufferContainer.buffer; };

    /******************** REQUESTS ********************/

    static void copy(wl_client *client, wl_resource *resource, wl_resource *buffer) noexcept;
    static void destroy(wl_client *client, wl_resource *resource) noexcept;

#if LOUVRE_SCREEN_COPY_MANAGER_VERSION >= 2
    static void copy_with_damage(wl_client *client, wl_resource *resource, wl_resource *buffer) noexcept;
#endif
    /******************** EVENTS ********************/

    // Since 1
    void buffer(UInt32 shmFormat, const LSize &size, UInt32 stride) noexcept;
    void flags(UInt32 flags) noexcept;
    void ready(UInt32 tvSecHi, UInt32 tvSecLow, UInt32 tvNsec) noexcept;
    void failed() noexcept;

    // Since 2
    bool damage(const LRect &rect) noexcept;
    bool damage(const LRegion &region) noexcept;

    // Since 3
    bool linuxDMABuf(UInt32 format, const LSize &size) noexcept;
    bool bufferDone() noexcept;

private:
    friend class GScreenCopyManager;
    friend class Louvre::LOutput;
    friend class Louvre::LScreenshotRequest;
    RScreenCopyFrame(Wayland::GOutput *outputRes, bool overlayCursor, const LRect &region, UInt32 id, Int32 version) noexcept;
    ~RScreenCopyFrame() noexcept;
    static void copyCommon(wl_resource *resource, wl_resource *buffer, bool waitForDamage) noexcept;
    LWeak<LOutput> m_output;

    struct BufferContainer
    {
        wl_listener onDestroy;
        wl_resource *buffer { nullptr };
    } m_bufferContainer;

    LScreenshotRequest m_frame;
    LRect m_rect, m_rectB;
    LSize m_initOutputModeSize;
    LSize m_initOutputSize;
    Int32 m_initOutputTransform;
    Int32 m_stride;
    LBitset<StateFlags> m_stateFlags;
};

#endif // RSCREENCOPYFRAME_H
