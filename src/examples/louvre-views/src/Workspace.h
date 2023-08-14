#ifndef WORKSPACE_H
#define WORKSPACE_H

#include <LLayerView.h>

class Output;

using namespace Louvre;

class Workspace : public LLayerView
{
public:
    Workspace(Output *output);

    Output *output = nullptr;
    std::list<Workspace*>outputLink;
};

#endif // WORKSPACE_H
