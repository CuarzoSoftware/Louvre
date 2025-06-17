#ifndef RSCREENCOPYFRAME_H
#define RSCREENCOPYFRAME_H

#include <CZ/Louvre/LScreenshotRequest.h>
#include <CZ/CZTransform.h>
#include <CZ/Louvre/LResource.h>
#include <CZ/CZBitset.h>
#include <CZ/CZWeak.h>
#include <CZ/skia/core/SkRect.h>

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

    GScreenCopyManager *screenCopyManagerRes() const noexcept { return m_screenCopyManagerRes; }
    LOutput *output()       const noexcept { return m_output; }
    const SkIRect &rect()     const noexcept { return m_rect; };
    const SkIRect &rectB()    const noexcept { return m_rectB; };
    bool compositeCursor()  const noexcept { return m_stateFlags.has(CompositeCursor); };
    bool alreadyUsed()      const noexcept { return m_stateFlags.has(AlreadyUsed); };
    bool waitForDamage()    const noexcept { return m_stateFlags.has(WaitForDamage); };
    bool accepted()         const noexcept { return m_stateFlags.has(Accepted); };
    wl_resource *buffer()   const noexcept { return m_bufferContainer.buffer; };

    /******************** REQUESTS ********************/

    static void copy(wl_client *client, wl_resource *resource, wl_resource *buffer) noexcept;
    static void destroy(wl_client *client, wl_resource *resource) noexcept;

#if LOUVRE_SCREEN_COPY_MANAGER_VERSION >= 2
    static void copy_with_damage(wl_client *client, wl_resource *resource, wl_resource *buffer) noexcept;
#endif
    /******************** EVENTS ********************/

    // Since 1
    void buffer(UInt32 shmFormat, const SkISize &size, UInt32 stride) noexcept;
    void flags(UInt32 flags) noexcept;
    void ready(const timespec &time) noexcept;
    void failed() noexcept;

    // Since 2
    bool damage(const SkIRect &rect) noexcept;
    bool damage(const SkRegion &region) noexcept;

    // Since 3
    bool linuxDMABuf(UInt32 format, const SkISize &size) noexcept;
    bool bufferDone() noexcept;

private:
    friend class GScreenCopyManager;
    friend class Louvre::LOutput;
    friend class Louvre::LScreenshotRequest;
    RScreenCopyFrame(GScreenCopyManager *screenCopyManagerRes, LOutput *output, bool overlayCursor, const SkIRect &region, UInt32 id, Int32 version) noexcept;
    ~RScreenCopyFrame() noexcept;
    static void copyCommon(wl_resource *resource, wl_resource *buffer, bool waitForDamage) noexcept;
    CZWeak<LOutput> m_output;
    CZWeak<GScreenCopyManager> m_screenCopyManagerRes;

    struct BufferContainer
    {
        wl_listener onDestroy;
        wl_resource *buffer { nullptr };
    } m_bufferContainer;

    LScreenshotRequest m_frame;
    SkIRect m_rect { 0, 0, 0, 0 }, m_rectB { 0, 0, 0, 0 };
    SkISize m_initOutputModeSize { 0, 0 };
    SkISize m_initOutputSize { 0, 0 };
    CZTransform m_initOutputTransform;
    Int32 m_stride;
    CZBitset<StateFlags> m_stateFlags;
};

#endif // RSCREENCOPYFRAME_H
