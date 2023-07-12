#include <private/LViewPrivate.h>
#include <private/LScenePrivate.h>

void LView::LViewPrivate::removeOutput(LView *view, LOutput *output)
{
    view->leftOutput(output);

    auto it = outputsMap.find(output);

    if (it != outputsMap.end())
        outputsMap.erase(it);

    if (!scene)
        return;

    auto sit = scene->imp()->outputsMap.find(output);

    if (sit != scene->imp()->outputsMap.end())
        scene->imp()->outputsMap.erase(sit);
}
