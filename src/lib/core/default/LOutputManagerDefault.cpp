#include <LOutputManager.h>
#include <LCompositor.h>
#include <LOutput.h>
#include <LCursor.h>
#include <LLog.h>

using namespace Louvre;

//! [outputPlugged]
void LOutputManager::outputPlugged(LOutput *connectedOutput)
{
    LLog::debug("Output %s connected.", connectedOutput->name());

    connectedOutput->setScale(connectedOutput->dpi() >= 120 ? 2 : 1);

    compositor()->addOutput(connectedOutput);

    // Organize outputs horizontally and sequentially.

    Int32 totalWidth = 0;

    for(LOutput *output : compositor()->outputs())
    {
        output->setPosC(LPoint(totalWidth,0));

        totalWidth += output->sizeC().w();
    }

    compositor()->repaintAllOutputs();
}
//! [outputPlugged]

//! [outputUnplugged]
void LOutputManager::outputUnplugged(LOutput *disconnectedOutput)
{
    compositor()->removeOutput(disconnectedOutput);

    // Organize outputs horizontally and sequentially.

    Int32 totalWidth = 0;

    if(!compositor()->outputManager()->outputs()->empty())
    {
        for(LOutput *output : compositor()->outputs())
        {
            output->setPosC(LPoint(totalWidth,0));
            totalWidth += output->sizeC().w();
        }

        compositor()->repaintAllOutputs();
    }
}
//! [outputUnplugged]
