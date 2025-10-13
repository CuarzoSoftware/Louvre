#include <CZ/Louvre/Private/LCompositorPrivate.h>
#include <CZ/Louvre/Protocols/BackgroundBlur/RBackgroundBlur.h>
#include <CZ/Louvre/Roles/LBackgroundBlur.h>
#include <CZ/Louvre/Roles/LSurface.h>
#include <CZ/Core/CZTime.h>

using namespace CZ;

LBackgroundBlur::LBackgroundBlur(const void *params) noexcept : LFactoryObject(FactoryObjectType), m_surface(*((LSurface*)params)) {}

Protocols::BackgroundBlur::RBackgroundBlur *LBackgroundBlur::backgroundBlurResource() const noexcept
{
    return m_backgroundBlurRes;
}

void LBackgroundBlur::handleCommit(bool sizeChanged) noexcept
{
    if (m_flags.has(Destroyed))
    {
        m_flags.remove(Destroyed);
        reset();
        return;
    }
    else if (supported())
    {
        fullPropsUpdate(sizeChanged);
    }
}

void LBackgroundBlur::fullPropsUpdate(bool sizeChanged) noexcept
{
    CZBitset<PropChanges> changesToNotify;

    changesToNotify.setFlag(RegionChanged,
        pendingProps().isEmpty != currentProps().isEmpty ||
        pendingProps().isFullSize != currentProps().isFullSize ||
        m_flags.has(RegionModified));

    if (!pendingProps().isEmpty)
    {
        if (pendingProps().isFullSize)
        {
            if (sizeChanged || pendingProps().isFullSize != currentProps().isFullSize)
            {
                pendingProps().region.setRect(
                    SkIRect::MakeSize(surface()->size()));

                changesToNotify.add(RegionChanged);
            }
        }
        else if (m_flags.has(RegionModified))
        {
            changesToNotify.add(RegionChanged);
            pendingProps().region.op(SkIRect::MakeSize(surface()->size()), SkRegion::kIntersect_Op);
        }
    }

    changesToNotify.setFlag(MaskChanged,
        pendingProps().maskType != currentProps().maskType ||
        m_flags.has(MaskModified));

    if (currentProps().colorScheme != pendingProps().colorScheme)
        changesToNotify.add(ColorSchemeChanged);

    if (currentProps().serial != pendingProps().serial)
    {
        changesToNotify.add(SerialChanged);

        if (currentProps().state != pendingProps().state)
            changesToNotify.add(StateChanged);
    }

    if (changesToNotify == 0)
        return;

    m_currentPropsIndex = 1 - m_currentPropsIndex;

    propsChanged(changesToNotify, pendingProps());
    pendingProps() = currentProps();
    m_flags.remove(RegionModified | MaskModified | SchemeModified);
}

void LBackgroundBlur::sendPendingConfiguration() noexcept
{
    if (!supported() || !m_flags.has(HasStateToSend))
        return;

    surface()->requestNextFrame(false);

    auto &res { *m_backgroundBlurRes };

    if (m_flags.has(HasStateToSend))
        res.state(m_pendingConfiguration.state);

    res.configure(m_pendingConfiguration.serial);

    m_sentConfigurations.push_back(m_pendingConfiguration);
    m_flags.remove(HasStateToSend);
}

void LBackgroundBlur::updateSerial() noexcept
{
    if (!m_flags.has(HasStateToSend))
    {
        m_pendingConfiguration.serial = CZTime::NextSerial();
        compositor()->imp()->unlockPoll();
    }
}

void LBackgroundBlur::reset() noexcept
{
    m_pendingConfiguration.serial++;
    m_pendingConfiguration.state = Disabled;
    pendingProps() = {};
    fullPropsUpdate(false);
}
