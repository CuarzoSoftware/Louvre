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

    if (output())
    {
        if (newSize.w() == 0)
            newSize.setW(output()->size().w());

        if (newSize.h() == 0)
            newSize.setH(output()->size().h());
    }

    configureSize(newSize);
}
