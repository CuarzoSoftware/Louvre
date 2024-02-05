#include <LTextureView.h>
#include "Workspace.h"
#include "Output.h"
#include "Global.h"
#include "Compositor.h"
#include "Topbar.h"
#include "Surface.h"

Workspace::Workspace(Output *output, Toplevel *toplevel, Workspace *prev) : LLayerView(output->workspacesContainer),
    output   { output },
    toplevel { toplevel }
{
    // The first workspace is the desktop
    if (output->workspaces.empty())
    {
        output->workspaces.push_back(this);
        outputLink = std::prev(output->workspaces.end());
        return;
    }

    // If fullscreen toplevel
    if (prev)
        outputLink = output->workspaces.insert(std::next(prev->outputLink), this);
    else
        outputLink = output->workspaces.insert(std::next(output->workspaces.begin()), this);

    output->updateWorkspacesPos();
}

Workspace::~Workspace()
{
    output->workspaces.erase(outputLink);
}

void Workspace::stealChildren()
{
    if (output->workspaces.front() == this)
    {
        if (output->wallpaperView)
        {
            output->wallpaperView->setParent(&background);
            output->wallpaperView->enableParentOffset(true);
        }

        LView *surfView;

        for (class Surface *surf : G::surfaces())
        {
            surfView = surf->getView();

            if (surfView && surfView->parent() == &G::compositor()->surfacesLayer && !surf->cursorRole())
            {
                if (G::mostIntersectedOuput(surfView) == output)
                {
                    surfView->setParent(&surfaces);
                    surfView->enableParentOffset(true);
                }
            }
        }

        if (output->topbar)
        {
            output->topbar->setParent(&overlay);
            output->topbar->enableParentOffset(true);
        }
    }
}

void Workspace::returnChildren()
{
    if (output->workspaces.front() == this)
    {
        G::enableClippingChildren(this, false);

        if (output->wallpaperView)
        {
            output->wallpaperView->setParent(&G::compositor()->backgroundLayer);
            output->wallpaperView->enableParentOffset(false);
        }

        LView *v;

        while (!surfaces.children().empty())
        {
            v = surfaces.children().front();
            v->setParent(&G::compositor()->surfacesLayer);
            v->enableParentOffset(false);
        }

        std::list<class Surface*>surfaces = G::surfaces();

        for (class Surface *s : surfaces)
            s->raise();

        if (output->topbar)
        {
            output->topbar->setParent(&G::compositor()->overlayLayer);
            output->topbar->enableParentOffset(false);
        }

        output->repaint();
    }
}

static void clipChildrenViews(LView *view, const LRect &rect)
{
    for (LView *child : view->children())
    {
        child->enableClipping(true);
        child->setClippingRect(rect);
        clipChildrenViews(child, rect);
    }
}

void Workspace::clipChildren()
{
    LRect rect;

    rect.setY(0);
    rect.setH(output->size().h());

    if (pos().x() + output->pos().x() < output->pos().x())
    {
        rect.setX(output->pos().x());
        rect.setW(output->size().w() + pos().x());
        if (rect.w() < 0)
            rect.setWidth(0);
    }
    else if (pos().x() + output->pos().x() > output->pos().x() + output->size().w())
    {
        rect.setX(output->size().w());
        rect.setW(0);
    }
    else
    {
        rect.setX(output->pos().x() + pos().x());
        rect.setW(output->size().w() - pos().x());
    }

    setSize(output->size());

    clipChildrenViews(this, rect);
}

Int32 Workspace::getIndex() const
{
    Int32 index = 0;

    for (Workspace *ws : output->workspaces)
    {
        if (ws == this)
            break;

        index++;
    }

    return index;
}
