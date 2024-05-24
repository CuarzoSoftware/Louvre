#include <LLayerRole.h>
#include <LCursor.h>
#include <LSurface.h>
#include <LCompositor.h>
#include <LSeat.h>
#include <LKeyboard.h>

using namespace Louvre;

void LLayerRole::atomsChanged(LBitset<AtomsChanges> changes, const Atoms &prevAtoms)
{
    L_UNUSED(prevAtoms);

    if (changes.check(AtomsChanges::KeyboardInteractivityChanged))
    {
        switch (keyboardInteractivity())
        {
        case KeyboardInteractivity::NoInteractivity:
            if (surface()->hasKeyboardFocus())
                seat()->keyboard()->setFocus(nullptr);
            if (surface()->hasKeyboardGrab())
                seat()->keyboard()->setGrab(nullptr);
            break;
        case KeyboardInteractivity::OnDemand:
            if (surface()->hasKeyboardGrab())
                seat()->keyboard()->setGrab(nullptr);
            break;
        case KeyboardInteractivity::Exclusive:
            seat()->keyboard()->setGrab(surface());
            break;
        }
    }

    if (exclusiveOutput())
        exclusiveOutput()->repaint();
}

void LLayerRole::configureRequest()
{
    if (!exclusiveOutput())
        setExclusiveOutput(cursor()->output());

    LSize newSize { size() };

    const LBitset<LEdge> edge { exclusiveZone().edge() };

    if (newSize.w() == 0 || edge == 0 || edge.check(LEdgeTop | LEdgeBottom))
        newSize.setW(exclusiveZone().rect().w());

    if (newSize.h() == 0 || edge == 0 || edge.check(LEdgeLeft | LEdgeRight))
        newSize.setH(exclusiveZone().rect().h());

    configureSize(newSize);
}
