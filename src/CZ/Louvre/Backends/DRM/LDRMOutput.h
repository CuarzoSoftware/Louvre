#ifndef LDRMOUTPUT_H
#define LDRMOUTPUT_H

#include "SRM.h"
#include <CZ/Louvre/Backends/LBackendOutput.h>

namespace CZ
{
class LDRMOutput : public LBackendOutput
{
public:
    static LOutput *Make(SRMConnector *conn) noexcept;

    bool init() noexcept override;
    bool repaint() noexcept override;
    void unit() noexcept override;

    const std::string &name() const noexcept override;
    const std::string &make() const noexcept override;
    const std::string &model() const noexcept override;
    const std::string &desc() const noexcept override;
    const std::string &serial() const noexcept override;

    SkISize mmSize() const noexcept override;
    RSubpixel subpixel() const noexcept override;
    bool isNonDesktop() const noexcept override;
    UInt32 id() const noexcept override;
    RDevice *device() const noexcept override;

    void setContentType(RContentType type) noexcept override;
    RContentType contentType() const noexcept override;

    /* Gamma LUT */

    size_t gammaSize() const noexcept override;
    std::shared_ptr<const RGammaLUT> gammaLUT() const noexcept override;
    bool setGammaLUT(std::shared_ptr<const RGammaLUT> gamma) noexcept override;

    /* Rendering */

    UInt32 imageIndex() const noexcept override;
    UInt32 imageAge() const noexcept override;
    const std::vector<std::shared_ptr<RImage>> &images() const noexcept override;
    void setDamage(const SkRegion &region) noexcept override;

    /* V-SYNC */

    bool canDisableVSync() const noexcept override;
    bool isVSyncEnabled() const noexcept override;
    bool enableVSync(bool enabled) noexcept override;
    UInt64 paintEventId() const noexcept override;

    /* Modes */

    const std::vector<std::shared_ptr<LOutputMode>> &modes() const noexcept override;
    const std::shared_ptr<LOutputMode> preferredMode() const noexcept override;
    const std::shared_ptr<LOutputMode> currentMode() const noexcept override;
    int setMode(std::shared_ptr<LOutputMode> mode) noexcept override;

    /* Cursor */

    bool hasCursor() const noexcept override;
    bool setCursor(UInt8 *pixels) noexcept override;
    bool setCursorPos(SkIPoint pos) noexcept override;

    SRMConnector *conn() const noexcept { return m_conn; }

    void handleInitializeGL() noexcept;
    void handlePaintGL() noexcept;
    void handleResizeGL() noexcept;
    void handlePresented(const CZPresentationTime &info) noexcept;
    void handleDiscarded(UInt64 paintEventId) noexcept;
    void handleUninitializeGL() noexcept;

private:
    LDRMOutput(SRMConnector *conn) noexcept;
    SRMConnector *m_conn;
    std::vector<std::shared_ptr<LOutputMode>> m_modes;
    std::string m_desc;
};
}

#endif // LDRMOUTPUT_H
