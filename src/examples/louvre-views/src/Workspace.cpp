#include "Workspace.h"
#include "Output.h"
#include "Global.h"
#include "Compositor.h"
#include "Topbar.h"
#include <LTextureView.h>
#include "Surface.h"

Workspace::Workspace(Output *output, Toplevel *toplevel) : LLayerView(output->workspacesContainer),
    background(this),
    surfaces(this),
    overlay(this)
{
    this->output = output;
    this->toplevel = toplevel;

    // The first workspace is the desktop
    if (output->workspaces.empty())
    {
        output->workspaces.push_back(this);
        outputLink = std::prev(output->workspaces.end());
        return;
    }

    // If fullscreen toplevel, inster right after desktop
    outputLink = output->workspaces.insert(std::next(output->workspaces.begin()), this);
    output->updateWorkspacesPos();
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
        LBox box;

        for (class Surface *surf : G::surfaces())
        {
            surfView = surf->getView();

            if (surfView && surfView->parent() == &G::compositor()->surfacesLayer)
            {
                box = surfView->boundingBox();

                if (LRect(box.x1, box.y1, box.x2 - box.x1, box.y2 - box.y1).intersects(output->rect()))
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

        if (output->topbar)
        {
            output->topbar->setParent(&G::compositor()->overlayLayer);
            output->topbar->enableParentOffset(false);
        }

        output->moveGL();
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
    setSize(output->size());
    LRect rect = LRect(pos(), size());
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
