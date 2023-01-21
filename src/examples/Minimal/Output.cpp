#include "Output.h"
#include <LPainter.h>
#include <LCompositor.h>
#include <LSeat.h>
#include <LDNDManager.h>
#include <LDNDIconRole.h>
#include <LCursor.h>
#include <Surface.h>

Output::Output() : LOutput()
{

}

void Output::initializeGL()
{
    //bg = LOpenGL::loadTexture("wallpaper.png");
    glClearColor(0.2,0.6,1,1);
}

void Output::paintGL()
{
    LPainter *p = painter();

    LPoint sp = posG();

    p->clearScreen();

    LRect currentRect;

    for(Surface *s : (list<Surface*>&)compositor()->surfaces())
    {
        if(!s->mapped() || s->minimized() || s->roleType() == LSurface::Role::Cursor || s->roleType() == LSurface::Role::Undefined || s->roleType() == LSurface::Role::DNDIcon)
            continue;


        currentRect = LRect(s->pos(true), s->size());

        if(currentRect != s->prevRect)
        {
            s->prevRect = currentRect;

            for(LOutput *o : compositor()->outputs())
            {
                if(o->rectG().intersects(currentRect))
                    s->sendOutputEnterEvent(o);
                else
                    s->sendOutputLeaveEvent(o);
            }
        }


        p->drawTexture(s->texture(),LRect(0,s->size(true)),LRect(s->pos(true)-sp, s->size()));

        s->requestNextFrame();
    }

    if(compositor()->seat()->dndManager()->icon())
    {
        LSurface *s = compositor()->seat()->dndManager()->icon()->surface();
        if(!s->mapped())
            return;
        s->setPos(compositor()->cursor()->position());
        p->drawTexture(s->texture(),
                       LRect(LPoint(),s->size(true)),LRect(s->pos(true)-sp,s->size()));
        s->requestNextFrame();
    }
}
