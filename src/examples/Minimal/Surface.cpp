#include "Surface.h"
#include <LCompositor.h>
#include <LCursor.h>
#include <LOutput.h>
#include <LSeat.h>
#include <LPointer.h>

Surface::Surface(Params *params, GLuint textureUnit) : LSurface(params, textureUnit) {}

void Surface::mappingChanged()
{   

    if(firstLaunch && mapped())
    {
        firstLaunch = false;

        LPoint outputPos = compositor()->cursor()->output()->posG();
        LSize outputSize = compositor()->cursor()->output()->sizeG();

        setPos(outputPos + outputSize/2 - size()/2);

        if(pos().x() < outputPos.x())
            setX(outputPos.x());
        if(pos().y() < outputPos.y())
            setY(outputPos.y());

    }

    compositor()->repaintAllOutputs();
}

