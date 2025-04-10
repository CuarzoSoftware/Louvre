#include "protocols/BackgroundBlur/background-blur.h"
#include <private/LCompositorPrivate.h>
#include <protocols/BackgroundBlur/RBackgroundBlur.h>
#include <LBackgroundBlur.h>
#include <LTime.h>

using namespace Louvre;

LBackgroundBlur::LBackgroundBlur(const void *params) noexcept : LFactoryObject(FactoryObjectType), m_surface(*((LSurface*)params))
{

}

LBackgroundBlur::~LBackgroundBlur() noexcept
{

}

Protocols::BackgroundBlur::RBackgroundBlur *LBackgroundBlur::backgroundBlurResource() const noexcept
{
    return m_backgroundBlurRes;
}

void LBackgroundBlur::configureRequest()
{
    configureState(Enabled);
    configureStyle(Light);
}

void LBackgroundBlur::propsChanged(LBitset<PropChanges> /*changes*/, const Props &/*prevProps*/)
{

}

void LBackgroundBlur::handleCommit(bool sizeChanged) noexcept
{
    if (m_flags.check(Destroyed))
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
    LBitset<PropChanges> changesToNotify;

    changesToNotify.setFlag(RegionOrPathChanged,
        pendingProps().isEmpty != currentProps().isEmpty ||
        pendingProps().isSvgPath != currentProps().isSvgPath ||
        pendingProps().isFullSize != currentProps().isFullSize);

    if (!pendingProps().isEmpty)
    {
        if (pendingProps().isFullSize)
        {
            if (sizeChanged || pendingProps().isFullSize != currentProps().isFullSize)
            {
                pendingProps().region.clear();
                pendingProps().region.addRect(LRect(LPoint(0, 0), surface()->size()));
                changesToNotify.add(RegionOrPathChanged);
            }
        }
        else
        {
            if (m_flags.check(AssignedRegionOrPath))
                changesToNotify.add(RegionOrPathChanged);

            if (pendingProps().isSvgPath)
            {
                // TODO: Currently it needs to be validated by the user
            }
            else if (m_flags.check(AssignedRegionOrPath) || sizeChanged)
            {
                const auto &bounds { pendingProps().region.extents() };

                if (bounds.x1 < 0 || bounds.y1 < 0 || bounds.x2 > surface()->size().w() || bounds.y2 > surface()->size().h())
                {
                    wl_resource_post_error(backgroundBlurResource()->resource(),
                                           BACKGROUND_BLUR_ERROR_OUT_OF_BOUNDS,
                                           "the region or path extends beyond the surface bounds");
                    return;
                }
            }
        }
    }

    if (currentProps().serial != pendingProps().serial)
    {
        changesToNotify.add(SerialChanged);

        if (currentProps().state != pendingProps().state)
            changesToNotify.add(StateChanged);

        if (currentProps().style != pendingProps().style)
            changesToNotify.add(StyleChanged);
    }

    if (changesToNotify == 0)
        return;

    m_currentPropsIndex = 1 - m_currentPropsIndex;

    propsChanged(changesToNotify, pendingProps());
    pendingProps() = currentProps();
    m_flags.remove(AssignedRegionOrPath);
}

void LBackgroundBlur::sendPendingConfiguration() noexcept
{
    if (!supported() || !m_flags.check(HasStateToSend | HasStyleToSend))
        return;

    surface()->requestNextFrame(false);

    auto &res { *m_backgroundBlurRes };

    if (m_flags.check(HasStateToSend))
        res.state(m_pendingConfiguration.state);

    if (m_flags.check(HasStyleToSend))
        res.style(m_pendingConfiguration.style);

    res.configure(m_pendingConfiguration.serial);

    m_sentConfigurations.push_back(m_pendingConfiguration);
    m_flags.remove(HasStateToSend | HasStyleToSend);
}

void LBackgroundBlur::updateSerial() noexcept
{
    if (!m_flags.check(HasStateToSend | HasStyleToSend))
    {
        m_pendingConfiguration.serial = LTime::nextSerial();
        compositor()->imp()->unlockPoll();
    }
}

void LBackgroundBlur::reset() noexcept
{
    m_pendingConfiguration.serial++;
    m_pendingConfiguration.state = Disabled;
    m_pendingConfiguration.style = Light;
    pendingProps().region.clear();
    pendingProps().svgPath.clear();
    pendingProps().isSvgPath = false;
    pendingProps().isEmpty = true;
    pendingProps().isFullSize = false;
    fullPropsUpdate(false);
}
