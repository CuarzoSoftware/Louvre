#include <LLayerRole.h>
#include <private/LSurfacePrivate.h>
#include <protocols/LayerShell/RLayerSurface.h>
#include <LTime.h>

using namespace Louvre;

void LLayerRole::atomicPropsChanged(LBitset<AtomicPropChanges> changes, const AtomicProps &prevAtomicProps)
{

}

void LLayerRole::handleSurfaceCommit(CommitOrigin origin) noexcept
{
    auto &res { *static_cast<LayerShell::RLayerSurface*>(resource()) };

    if (m_flags.check(HasPendingLayer))
    {
        m_flags.remove(HasPendingLayer);
        surface()->imp()->setLayer(pendingProps().layer);
    }

    printf("LAYER %d\n", pendingProps().layer);


    if (surface()->mapped())
    {
        if (!surface()->buffer())
            surface()->imp()->setMapped(false);
    }
    else
    {
        if (surface()->buffer())
            surface()->imp()->setMapped(true);
        else
        {
            LSize size;

            if (output())
            {
                size = output()->size();
            }
            res.configure(LTime::nextSerial(), size);
        }
    }
}
