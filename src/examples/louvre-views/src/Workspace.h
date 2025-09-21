#ifndef WORKSPACE_H
#define WORKSPACE_H

#include <LLayerView.h>

class Output;
class Toplevel;

using namespace Louvre;

class Workspace : public LLayerView {
 public:
  Workspace(Output *output, Toplevel *toplevel = nullptr,
            Workspace *prev = nullptr);
  ~Workspace();

  std::list<Workspace *>::iterator outputLink;
  Output *output{nullptr};

  LLayerView background{this};
  LLayerView surfaces{this};
  LLayerView overlay{this};

  Toplevel *toplevel{nullptr};

  // Moves the main layers childrens into this
  void stealChildren();
  void returnChildren();
  void clipChildren();
  void show(bool show);

  Int32 getIndex() const;
};

#endif  // WORKSPACE_H
