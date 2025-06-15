#include <CZ/Louvre/Private/LCompositorPrivate.h>
#include <CZ/Louvre/Protocols/BackgroundBlur/RBackgroundBlur.h>
#include <LBackgroundBlur.h>
#include <LTime.h>

using namespace Louvre;

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
        pendingProps().isFullSize != currentProps().isFullSize);

    if (!pendingProps().isEmpty)
    {
        if (pendingProps().isFullSize)
        {
            if (sizeChanged || pendingProps().isFullSize != currentProps().isFullSize)
            {
                pendingProps().region.clear();
                pendingProps().region.addRect(LRect(LPoint(0, 0), surface()->size()));
                changesToNotify.add(RegionChanged);
            }
        }
        else if (m_flags.has(RegionModified))
        {
            changesToNotify.add(RegionChanged);
            pendingProps().region.clip(LPoint(0, 0), surface()->size());
        }
    }

    changesToNotify.setFlag(MaskChanged,
        pendingProps().maskType != currentProps().maskType ||
        m_flags.has(MaskModified));

    if (currentProps().serial != pendingProps().serial)
    {
        changesToNotify.add(SerialChanged);

        if (currentProps().state != pendingProps().state)
            changesToNotify.add(StateChanged);

        if (currentProps().colorHint != pendingProps().colorHint)
            changesToNotify.add(ColorHintChanged);
    }

    if (changesToNotify == 0)
        return;

    m_currentPropsIndex = 1 - m_currentPropsIndex;

    propsChanged(changesToNotify, pendingProps());
    pendingProps() = currentProps();
    m_flags.remove(RegionModified | MaskModified);
}

void LBackgroundBlur::sendPendingConfiguration() noexcept
{
    if (!supported() || !m_flags.has(HasStateToSend | HasColorHintToSend))
        return;

    surface()->requestNextFrame(false);

    auto &res { *m_backgroundBlurRes };

    if (m_flags.has(HasStateToSend))
        res.state(m_pendingConfiguration.state);

    if (m_flags.has(HasColorHintToSend))
        res.colorHint(m_pendingConfiguration.colorHint);

    res.configure(m_pendingConfiguration.serial);

    m_sentConfigurations.push_back(m_pendingConfiguration);
    m_flags.remove(HasStateToSend | HasColorHintToSend);
}

void LBackgroundBlur::updateSerial() noexcept
{
    if (!m_flags.has(HasStateToSend | HasColorHintToSend))
    {
        m_pendingConfiguration.serial = LTime::nextSerial();
        compositor()->imp()->unlockPoll();
    }
}

void LBackgroundBlur::reset() noexcept
{
    m_pendingConfiguration.serial++;
    m_pendingConfiguration.state = Disabled;
    m_pendingConfiguration.colorHint = Unknown;
    pendingProps().region.clear();
    pendingProps().svgPathMask.clear();
    pendingProps().roundRectMask = LRRect();
    pendingProps().maskType = NoMask;
    pendingProps().isEmpty = true;
    pendingProps().isFullSize = false;
    fullPropsUpdate(false);
}
