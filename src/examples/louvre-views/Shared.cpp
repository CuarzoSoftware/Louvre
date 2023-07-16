#include <LOutput.h>
#include <LLog.h>
#include <Shared.h>
#include <Compositor.h>

static SharedData data;

Compositor *comp()
{
    return (Compositor*)LCompositor::compositor();
}

std::list<Output *> &outps()
{
    return (std::list<Output*>&)comp()->outputs();
}

Int32 minimizedItemHeight()
{
    return 48*comp()->globalScale();
}

void arrangeOutputs()
{
    // Use the Output Manager to get avaliable outputs
    if (comp()->seat()->outputs()->empty())
    {
        LLog::fatal("No output available.");
        comp()->finish();
    }

    Int32 futureGlobalScale = 1;

    // Set double scale to outputs with DPI >= 120
    for (LOutput *output : *comp()->seat()->outputs())
    {
        if (output->dpi() >= 120)
        {
            output->setScale(2);
            futureGlobalScale = 2;
        }
    }

    Int32 totalWidth = 0;

    // Organize outputs horizontally and sequentially.
    for (LOutput *output : *comp()->seat()->outputs())
    {
        output->setPosC(LPoint(totalWidth, 0));

        if (output->scale() == futureGlobalScale)
            totalWidth += output->sizeB().w();
        else
            totalWidth += output->sizeB().w()*futureGlobalScale;

        comp()->addOutput(output);
        output->repaint();
    }
}

SharedData *shared()
{
    return &data;
}
