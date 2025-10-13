#ifndef LBACKENDOUTPUT_H
#define LBACKENDOUTPUT_H

#include <CZ/Ream/RContentType.h>
#include <CZ/Ream/RSubpixel.h>
#include <CZ/Ream/RImage.h>
#include <CZ/Louvre/LObject.h>
#include <string>

class CZ::LBackendOutput : public LObject
{
public:
    LOutput *output() const noexcept { return m_output; }

    virtual bool init() noexcept = 0;
    virtual bool repaint() noexcept = 0;
    virtual void unit() noexcept = 0;

    virtual const std::string &name() const noexcept = 0;
    virtual const std::string &make() const noexcept = 0;
    virtual const std::string &model() const noexcept = 0;
    virtual const std::string &desc() const noexcept = 0;
    virtual const std::string &serial() const noexcept = 0;

    virtual SkISize mmSize() const noexcept = 0;
    virtual RSubpixel subpixel() const noexcept = 0;
    virtual bool isNonDesktop() const noexcept = 0;
    virtual UInt32 id() const noexcept = 0;
    virtual RDevice *device() const noexcept = 0;

    virtual void setContentType(RContentType type) noexcept = 0;
    virtual RContentType contentType() const noexcept = 0;

    /* Gamma LUT */

    virtual size_t gammaSize() const noexcept = 0;
    virtual std::shared_ptr<const RGammaLUT> gammaLUT() const noexcept = 0;
    virtual bool setGammaLUT(std::shared_ptr<const RGammaLUT> gamma) noexcept = 0;

    /* Rendering */

    virtual UInt32 imageIndex() const noexcept = 0;
    virtual UInt32 imageAge() const noexcept = 0;
    virtual const std::vector<std::shared_ptr<RImage>> &images() const noexcept = 0;
    virtual void setDamage(const SkRegion &region) noexcept = 0; // In RImage-local coords

    /* V-SYNC */

    virtual bool canDisableVSync() const noexcept = 0;
    virtual bool isVSyncEnabled() const noexcept = 0;
    virtual bool enableVSync(bool enabled) noexcept = 0;
    virtual UInt64 paintEventId() const noexcept = 0;

    /* Modes */

    virtual const std::vector<std::shared_ptr<LOutputMode>> &modes() const noexcept = 0;
    virtual const std::shared_ptr<LOutputMode> preferredMode() const noexcept = 0;
    virtual const std::shared_ptr<LOutputMode> currentMode() const noexcept = 0;
    // 1 success, 0 rollback, -1 dead
    virtual int setMode(std::shared_ptr<LOutputMode> mode) noexcept = 0;

    /* Cursor */

    virtual bool hasCursor() const noexcept = 0;
    virtual bool setCursor(UInt8 *pixels) noexcept = 0;
    virtual bool setCursorPos(SkIPoint pos) noexcept = 0;
protected:
    friend class LOutput;
    CZWeak<LOutput> m_output;
};

#endif // LBACKENDOUTPUT_H
