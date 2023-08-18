#ifndef WORKSPACE_H
#define WORKSPACE_H

#include <LLayerView.h>

class Output;
class Toplevel;

using namespace Louvre;

class Workspace : public LLayerView
{
public:
    Workspace(Output *output, Toplevel *toplevel = nullptr);

    Output *output = nullptr;
    std::list<Workspace*>::iterator outputLink;

    LLayerView background, surfaces, overlay;

    Toplevel *toplevel = nullptr;

    // Moves the main layers childrens into this
    void stealChildren();
    void returnChildren();
    void clipChildren();

    Int32 getIndex() const;
};

#endif // WORKSPACE_H
