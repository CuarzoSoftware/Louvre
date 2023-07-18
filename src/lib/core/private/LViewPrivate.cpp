#include <private/LViewPrivate.h>
#include <private/LScenePrivate.h>
#include <private/LSceneViewPrivate.h>

void LView::LViewPrivate::removeOutput(LView *view, LOutput *output)
{
    view->leftOutput(output);

    auto it = outputsMap.find(output);

    if (it != outputsMap.end())
        outputsMap.erase(it);

    if (view->type() != Scene)
        return;

    LSceneView *sceneView = (LSceneView*)view;

    auto sit = sceneView->imp()->outputsMap.find(output);

    if (sit != sceneView->imp()->outputsMap.end())
        sceneView->imp()->outputsMap.erase(sit);
}
