#ifndef LOFFSCREENOUTPUT_H
#define LOFFSCREENOUTPUT_H

#include <CZ/Louvre/Backends/LBackendOutput.h>
#include <CZ/Core/CZPresentationTime.h>
#include <future>
#include <semaphore>

namespace CZ
{
class LOffscreenOutput : public LBackendOutput
{
public:
    LOffscreenOutput(LOffscreenBackend *backend, SkISize modeSize) noexcept;
    ~LOffscreenOutput() noexcept;

    bool init() noexcept override;
    bool repaint() noexcept override;
    void unit() noexcept override;

    const std::string &name() const noexcept override { return info.name; };
    const std::string &make() const noexcept override { return info.make; };
    const std::string &model() const noexcept override { return info.model; };
    const std::string &desc() const noexcept override { return info.desc; };
    const std::string &serial() const noexcept override { return info.serial; };

    SkISize mmSize() const noexcept override { return {0,0}; }
    RSubpixel subpixel() const noexcept override { return RSubpixel::Unknown; }
    bool isNonDesktop() const noexcept override { return false; };
    UInt32 id() const noexcept override { return info.id; };
    RDevice *device() const noexcept override;

    void setContentType(RContentType /*type*/) noexcept override {};
    RContentType contentType() const noexcept override { return RContentType::Graphics; };

    /* Gamma LUT */

    size_t gammaSize() const noexcept override { return 0; };
    std::shared_ptr<const RGammaLUT> gammaLUT() const noexcept override { return {}; };
    bool setGammaLUT(std::shared_ptr<const RGammaLUT> /*gamma*/) noexcept override { return false; };

    /* Rendering */

    UInt32 imageIndex() const noexcept override { return 0; };
    UInt32 imageAge() const noexcept override { return info.age; }
    const std::vector<std::shared_ptr<RImage>> &images() const noexcept override { return info.images; };
    void setDamage(const SkRegion &region) noexcept override { CZ_UNUSED(region) }

    /* V-SYNC */

    bool canDisableVSync() const noexcept override { return false; };
    bool isVSyncEnabled() const noexcept override { return true; };
    bool enableVSync(bool /*enabled*/) noexcept override { return false; };
    UInt64 paintEventId() const noexcept override { return info.time.paintEventId; };

    /* Modes */

    const std::vector<std::shared_ptr<LOutputMode>> &modes() const noexcept override { return info.modes; };
    const std::shared_ptr<LOutputMode> preferredMode() const noexcept override { return info.modes[0]; };
    const std::shared_ptr<LOutputMode> currentMode() const noexcept override { return info.modes[0]; };
    int setMode(std::shared_ptr<LOutputMode> /*mode*/) noexcept override { return 1; };

    /* Cursor */

    bool hasCursor() const noexcept override { return false; };
    bool setCursor(UInt8 */*pixels*/) noexcept override { return false; };
    bool setCursorPos(SkIPoint /*pos*/) noexcept override { return false; };

    struct
    {
        UInt32 id;
        std::string name;
        std::string make;
        std::string model;
        std::string desc;
        std::string serial;
        std::vector<std::shared_ptr<RImage>> images { {} };
        std::vector<std::shared_ptr<LOutputMode>> modes;
        UInt32 age { 0 };
        CZPresentationTime time {};
    } info {};

    CZWeak<LOffscreenBackend> backend;
    std::binary_semaphore semaphore { 0 };
    std::optional<std::promise<bool>> unitPromise;
    bool pendingRepaint { false };
};
}
#endif // LOFFSCREENOUTPUT_H
