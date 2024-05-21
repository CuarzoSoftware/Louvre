#include <LLayerRole.h>
#include <LCursor.h>

using namespace Louvre;

void LLayerRole::atomicPropsChanged(LBitset<AtomicPropChanges> changes, const AtomicProps &prevAtomicProps)
{

}

void LLayerRole::configureRequest()
{
    if (!output())
        setOutput(cursor()->output());

    LSize newSize { size() };

    const LBitset<LEdge> edge { exclusiveZone().edge() };

    if (newSize.w() == 0 || edge == 0 || edge.check(LEdgeTop | LEdgeBottom))
        newSize.setW(exclusiveZone().rect().w());

    if (newSize.h() == 0 || edge == 0 || edge.check(LEdgeLeft | LEdgeRight))
        newSize.setH(exclusiveZone().rect().h());

    configureSize(newSize);
}
