#ifndef RSCREENCOPYFRAME_H
#define RSCREENCOPYFRAME_H

#include <LScreenCopyFrame.h>
#include <LResource.h>
#include <LWeak.h>
#include <LRect.h>

class Louvre::Protocols::ScreenCopy::RScreenCopyFrame final : public LResource
{
public:    
    Wayland::GOutput *outputRes() const noexcept { return m_outputRes.get(); }
    const LRect &region() const noexcept { return m_region; };
    bool overlayCursor() const noexcept { return m_overlayCursor; };

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

    // Since 3
    bool linuxDMABuf(UInt32 format, const LSize &size) noexcept;
    bool bufferDone() noexcept;

private:
    friend class GScreenCopyManager;
    friend class Louvre::LScreenCopyFrame;
    friend class Louvre::LOutput;
    RScreenCopyFrame(Wayland::GOutput *outputRes, bool overlayCursor, const LRect &region, UInt32 id, Int32 version) noexcept;
    ~RScreenCopyFrame() noexcept;
    LWeak<Wayland::GOutput> m_outputRes;

    struct BufferContainer
    {
        wl_listener onBufferDestroyListener;
        wl_resource *m_buffer;
    } m_bufferContainer;

    LScreenCopyFrame m_frame;
    wl_listener m_onBufferDestroy;
    LRect m_region;
    LSize m_sentBufferSize;
    Int32 m_sentBufferStride;
    bool m_overlayCursor;
    bool m_used { false };
    bool m_handled { false };
    bool m_waitForDamage;
};

#endif // RSCREENCOPYFRAME_H
