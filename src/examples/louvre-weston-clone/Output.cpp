#include "Output.h"
#include "LLog.h"
#include <Compositor.h>
#include <LPainter.h>
#include <Surface.h>
#include <LToplevelRole.h>
#include <unistd.h>
#include <LTime.h>
#include <LCursor.h>

Output::Output():LOutput(){}

void Output::addExposedRect(const LRect &rect)
{
    exposedRegionG[0].addRect(rect);
    exposedRegionG[1].addRect(rect);
}

void Output::initializeGL()
{
    fullRefresh = true;
    first[0] = true;
    first[1] = true;
}

void Output::paintGL()
{

    LLog::debug("Current buff %d", currentBuffer());

    // Si TRUE vuelve a pintar ambos buffers
    if(fullRefresh)
    {
        fullRefresh = false;
        exposedRegionG[0].clear();
        exposedRegionG[1].clear();

        // Daña todo el área de los framebuffers
        exposedRegionG[0].addRect(rectC());
        exposedRegionG[1].addRect(rectC());
    }

    // Variables temporales para reducir verbosidad
    Compositor *c = (Compositor*)compositor();
    LPainter *p = painter();

    // Obtiene el índice del buffer actual y anterior (pueden ser 0 FRONT o 1 BACK)
    Int32 currBuff = currentBuffer();
    Int32 prevBuff = 1 - currBuff;

    // Parsea la lista de superficies del compositor (LSurface -> Surface)
    list<Surface*> &surfaces = (list<Surface*>&)compositor()->surfaces();

    // Si hay una superficie en modo fullscreen solo debemos repintar las superficies posteriores a ella
    // por lo que esta variable permite omitir superficies en el loop siguiente
    bool passFullscreen = false;

    // Calcula los daños de las superficies
    for(list<Surface*>::iterator it = surfaces.begin(); it != surfaces.end(); it++)
    {
        Surface *s = *it;

        // Saltamos superficies que no nos interesan
        if(!s->mapped() || s->roleId() == LSurface::Role::Cursor || s->roleId() == LSurface::Role::Undefined || s->minimized())
            continue;

        // Si se encontró la superficie fullscreen
        if(s == c->fullscreenSurface)
            passFullscreen = true;

        // Si existe una sup fullscreen pero aún no se encuentra skip
        if(c->fullscreenSurface && ! passFullscreen)
            continue;

        // Calcula la recta actual de la superficie
        LPoint sp = s->rolePosC();

        LRect rG = LRect(sp,s->sizeC());

        // Actualiza las salidas que intersecta
        for(LOutput *o : compositor()->outputs())
        {
            if(o->rectC().intersects(rG))
            {
                s->sendOutputEnterEvent(o);

                // Añade la salida al hash si no existe
                if(s->outputParams.find(this) == s->outputParams.end())
                    s->outputParams[this] = Surface::OutputParams();
            }
            else
            {
                s->sendOutputLeaveEvent(o);
            }
        }

        // Almacenamos los datos de la superficie de la salida actual
        s->cp = &s->outputParams[this];

        s->cp->bufferScaleMatchGlobalScale = s->bufferScale() == compositor()->globalScale();

        // Verifica si su posición o tamaño ha cambiado desde la última vez que renderizó el frame actual (2 frames antes)
        bool rectChanged = rG != s->cp->rectC[currBuff];

        // Limpia el daño total de la superficie
        s->cp->totalDamagesG.clear();

        // Almacena una variable indicando si hay daños para utilizarla en el prox frame
        s->cp->changed[currBuff] = s->hasDamage();

        // Si cambió su tamaño, posición u orden en la lista
        if(rectChanged || s->cp->changedOrder[currBuff])
        {

            // Daña toda la superficie
            s->cp->totalDamagesG.addRect(rG);

            // Añade la parte que quedó expuesta a una región temporal
            LRegion exposed;
            exposed.addRect(s->cp->rectC[currBuff]);

            // Si no cambió de orden se resta la recta actual de la superficie a la región temporal
            if(!s->cp->changedOrder[currBuff])
                exposed.subtractRect(rG);

            // Añade la región temporal a la región expuesta de este frame
            exposedRegionG[currBuff].addRegion(exposed);

            // Falso hasta que la superficie vuelva a cambiar de orden en el stack
            s->cp->changedOrder[currBuff] = false;

            // Guarda la recta actual de la superficie en este frame
            s->cp->rectC[currBuff] = rG;

            // Guarda los daños actuales de la superficie para el prox frame
            s->cp->damagesG[currBuff].clear();
            s->cp->damagesG[currBuff].addRect(LRect(0,s->sizeC()));

        }

        // Si la superficie no cambió de posición, tamaño u orden en el stack
        else
        {

            // Asigna los daños actuales a los totales de la superficie en este frame
            s->cp->totalDamagesG = s->damagesC();

            // Añade los daños anteriores al total actual
            s->cp->totalDamagesG.addRegion(s->cp->damagesG[prevBuff]);

            // Guarda los daños actuales de la superficie para el prox frame
            s->cp->damagesG[currBuff] = s->damagesC();

            // Añade el offset de la posición actual de la superficie a los daños totales de la superficie
            s->cp->totalDamagesG.offset(rG.pos());

        }

    }

    // Si el cursor no soporta composición por HW, añadimos su recta a la región expuesta
    if(!compositor()->cursor()->hasHardwareSupport(this))
    {
        exposedRegionG[currBuff].addRect(cursorRectG[currBuff]);
        exposedRegionG[currBuff].addRect(compositor()->cursor()->rectC());
        cursorRectG[currBuff] = compositor()->cursor()->rectC();
    }

    // Va almacenando la suma de todas las regiones opacas de las superficies
    LRegion opaqueDamagesG;

    glDisable(GL_BLEND);

    // Renderizamos los daños opacos desde el frente al fondo
    for(list<Surface*>::reverse_iterator it = surfaces.rbegin(); it != surfaces.rend(); it++)
    {
        Surface *s = *it;

        // Saltamos superficies que no nos interesan
        if(!s->mapped() || s->roleId() == LSurface::Role::Cursor || s->roleId() == LSurface::Role::Undefined || s->minimized())
            continue;

        // Almacenamos recta actual de la superficie en una variable mas cortita
        LRect &rG = s->cp->rectC[currBuff];

        // Copiamos su región opaca y le añadimos el offset de su pos
        s->cp->temporalOpaqueRegionG = s->opaqueRegionC();
        s->cp->temporalOpaqueRegionG.offset(rG.pos());

        // Copiamos su región transparente y le añadimos el offset de su pos
        s->cp->temporalTranslucentRegionG = s->translucentRegionC();
        s->cp->temporalTranslucentRegionG.offset(rG.pos());

        // Añadimos la parte expuesta de superficies superiores a el daño actual
        s->cp->totalDamagesG.addRegion(exposedRegionG[currBuff]);
        s->cp->totalDamagesG.clip(rG);

        // Creamos una region con el tamaño actual de la sup, luego le restamos la zonas opacas de superficies superiores para ver si es visible [1]
        LRegion hidden;
        hidden.addRect(rG);

        // Resta las regiones opacas de superficies superiores
        s->cp->totalDamagesG.subtractRegion(opaqueDamagesG);
        hidden.subtractRegion(opaqueDamagesG);

         UInt64 tot = s->allOutputsRequestedNewFrame();

        // [1] Le pedimos que renderice el prox frame solo si está visible y si otras superficies ya utilizaron sus daños
        if(!hidden.empty() || (tot > 0 && compositor()->outputs().size() > 1))
        {
            s->cp->requestedNewFrame = true;
        }

        tot = s->allOutputsRequestedNewFrame();

        if(tot == s->outputs().size())
        {
            // Si todas las salidas ya ocuparon sus daños, las marcamos otra vez como false
            for(LOutput *o : s->outputs())
            {
                s->outputParams[o].requestedNewFrame = false;
            }

            s->requestNextFrame();
        }


        // Intersectamos las regiones opacas con los daños totales
        s->cp->totalOpaqueDamagesG = s->cp->temporalOpaqueRegionG;
        s->cp->totalOpaqueDamagesG.intersectRegion(s->cp->totalDamagesG);

        // Intersectamos las regiones transparentes con los daños totales
        s->cp->totalTranslucentDamagesG = s->cp->temporalTranslucentRegionG;
        s->cp->totalTranslucentDamagesG.intersectRegion(s->cp->totalDamagesG);

        // Añadimos la región opaca a la suma de regiones opacas de todas las superficies superiores
        opaqueDamagesG.addRegion(s->cp->temporalOpaqueRegionG);

        // Añadimos la región transparente dañada a la región expuesta
        exposedRegionG[currBuff].addRegion(s->cp->totalTranslucentDamagesG);
        exposedRegionG[currBuff].subtractRegion(opaqueDamagesG);

        // Dibujamos las zonas dañadas opacas


        if(s->cp->bufferScaleMatchGlobalScale)
        {
            for(const LRect &d : s->cp->totalOpaqueDamagesG.rects())
            {
                p->drawTextureC(
                            s->texture(),
                            LRect(d.pos()-s->cp->rectC[currBuff].pos(), d.size()),
                            d);
            }
        }

        else
        {
            for(const LRect &d : s->cp->totalOpaqueDamagesG.rects())
            {
                p->drawTextureC(
                            s->texture(),
                            (LRect(d.pos()-s->cp->rectC[currBuff].pos(), d.size()) * s->bufferScale()) / compositor()->globalScale(),
                            d);
            }
        }


        // Si se llega a la superficie fullscreen no hace falta calcular las demás superficies (están todas detrás)
        if(s == c->fullscreenSurface)
            break;

    }


    // Recortamos la región expuesta a la recta de la pantalla
    exposedRegionG[currBuff].clip(rectC());

    // Dibujamos las partes expuestas del fondo
    if(!c->fullscreenSurface)
    {
        for(const LRect &d : exposedRegionG[currBuff].rects())
        {
            //p->drawTexture(backgroundTexture,d*scale(),d);
            p->drawColorC(d,0.3,0.3,0.7,1);
        }
    }


    // Ahora dibujamos las zonas transparentes
    glEnable(GL_BLEND);

    // Guarda todas las regiones renderizadas del fondo hacia el frente (hay que repintar esas partes)
    LRegion totalRenderedG;
    totalRenderedG = exposedRegionG[currBuff];

    passFullscreen = false;

    for(Surface *s : surfaces)
    {
        if(!s->mapped() || s->roleId() == LSurface::Role::Cursor || s->roleId() == LSurface::Role::Undefined || s->minimized())
            continue;

        if(s == c->fullscreenSurface)
            passFullscreen = true;

        if(c->fullscreenSurface && ! passFullscreen)
            continue;

        s->cp->totalTranslucentDamagesG.addRegion(totalRenderedG);
        s->cp->totalTranslucentDamagesG.intersectRegion(s->cp->temporalTranslucentRegionG);
        totalRenderedG.addRegion(s->cp->totalTranslucentDamagesG);
        totalRenderedG.addRegion(s->cp->totalOpaqueDamagesG);


        // Draw transulcent rects
        if(s->cp->bufferScaleMatchGlobalScale)
        {
            for(const LRect &d : s->cp->totalTranslucentDamagesG.rects())
                p->drawTextureC(
                            s->texture(),
                            LRect(d.pos() - s->cp->rectC[currBuff].pos(),d.size()),
                            d);
        }
        else
        {
            for(const LRect &d : s->cp->totalTranslucentDamagesG.rects())
                p->drawTextureC(
                            s->texture(),
                            (LRect(d.pos() - s->cp->rectC[currBuff].pos(),d.size())*s->bufferScale())/compositor()->globalScale(),
                            d);
        }

    }  


    // Dibujamos la barra superior
    if(first[currBuff])
    {
        p->drawColorC(LRect(rectC().x(),rectC().y(),rectC().w(),32*compositor()->globalScale()),1,1,1,0.75);
        first[currBuff] = false;
    }
    else if(!c->fullscreenSurface)
    {
        exposedRegionG[currBuff].subtractRect(LRect(rectC().x(),rectC().y() + 32*compositor()->globalScale(),rectC().w(),rectC().h()));

        for(const LRect &d : exposedRegionG[currBuff].rects())
            p->drawColorC(d,1,1,1,0.75);
    }


    exposedRegionG[currBuff].clear();


}
