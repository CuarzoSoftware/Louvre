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

void LBackgroundBlur::atomsChanged(LBitset<AtomChanges> /*changes*/, const Atoms &/*prevAtoms*/)
{

}

void LBackgroundBlur::handleCommit() noexcept
{
    if (m_flags.check(Destroyed))
    {
        m_flags.remove(Destroyed);
        reset();
        return;
    }
    else if (supported())
    {
        fullAtomsUpdate();
    }
}

void LBackgroundBlur::fullAtomsUpdate()
{
    LBitset<AtomChanges> changesToNotify;

    if (m_flags.check(AssignedRegion))
        changesToNotify.add(RegionChanged);

    if (currentAtoms().serial != pendingAtoms().serial)
    {
        changesToNotify.add(SerialChanged);

        if (currentAtoms().state != pendingAtoms().state)
            changesToNotify.add(StateChanged);

        if (currentAtoms().style != pendingAtoms().style)
            changesToNotify.add(StyleChanged);
    }

    if (changesToNotify == 0)
        return;

    m_currentAtomsIndex = 1 - m_currentAtomsIndex;

    atomsChanged(changesToNotify, pendingAtoms());

    if (m_flags.check(AssignedRegion))
    {
        if (currentAtoms().region)
            pendingAtoms().region.reset(new LRegion(*currentAtoms().region.get()));
        else
            pendingAtoms().region.reset();

        if (currentAtoms().path)
            pendingAtoms().path.reset(new std::string(*currentAtoms().path.get()));
        else
            pendingAtoms().path.reset();
    }

    if (changesToNotify.check(SerialChanged))
    {
        pendingAtoms().serial = currentAtoms().serial;

        if (changesToNotify.check(StateChanged))
            pendingAtoms().state = currentAtoms().state;

        if (changesToNotify.check(StyleChanged))
            pendingAtoms().style = currentAtoms().style;
    }

    m_flags.remove(AssignedRegion);
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

    if (region())
    {
        m_flags.set(AssignedRegion);
        pendingAtoms().region.reset();
    }

    if (path())
    {
        m_flags.set(AssignedRegion);
        pendingAtoms().path.reset();
    }

    fullAtomsUpdate();
}
