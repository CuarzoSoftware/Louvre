#ifndef COMPOSITOR_H
#define COMPOSITOR_H

#include <LTextureView.h>
#include <LSceneView.h>
#include <LLayerView.h>
#include <LCompositor.h>
#include <LScene.h>
#include <LView.h>
#include <LTimer.h>

using namespace Louvre;

class Tooltip;
class Client;

class Compositor : public LCompositor
{
public:
    void initialized() override;
    void uninitialized() override;

    LFactoryObject *createObjectRequest(LFactoryObject::Type type, const void *params) override;
    void onAnticipatedObjectDestruction(LFactoryObject *object) override;
    void fadeOutSurface(LBaseSurfaceRole *role, UInt32 ms);

    // Global scene used to render all outputs
    LScene scene {};
    LLayerView backgroundLayer { scene.mainView() };
    LLayerView surfacesLayer { scene.mainView() };
    LLayerView workspacesLayer { scene.mainView() };
    LLayerView fullscreenLayer { scene.mainView() };
    LLayerView overlayLayer { scene.mainView() };
    LLayerView tooltipsLayer { scene.mainView() };
    LLayerView cursorLayer { scene.mainView() };

    // Timer for updating the clock every minute
    LTimer clockMinuteTimer;
    static Int32 millisecondsUntilNextMinute();

    // Shared texture used in all clock views
    LTexture *clockTexture = nullptr;
    LTexture *oversamplingLabelTexture = nullptr;
    LTexture *vSyncLabelTexture = nullptr;

    // If true, we call scene->handlePointerEvent() once before scene->handlePaintGL().
    // The reason for this is that pointer events are only emitted when the pointer itself moves,
    // and they do not trigger when, for example, a view moves and positions itself under the cursor.
    // As a result, this call updates focus, cursor texture and so on...in those cases
    bool updatePointerBeforePaint = false;

    // Turns black all outputs until an output unplug is finished
    bool outputUnplugHandled = true;
    bool checkUpdateOutputUnplug();

    pid_t wofiPID { -1 };
    LWeak<Client> wofiClient;
};

#endif // COMPOSITOR_H
