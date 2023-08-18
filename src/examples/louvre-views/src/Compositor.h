#ifndef COMPOSITOR_H
#define COMPOSITOR_H

#include <LLayerView.h>
#include <LCompositor.h>
#include <LScene.h>
#include <LView.h>

using namespace Louvre;

class Tooltip;

class Compositor : public LCompositor
{
public:
    Compositor();
    ~Compositor();

    void initialized() override;

    LClient *createClientRequest(LClient::Params *params) override;
    LOutput *createOutputRequest() override;
    LSurface *createSurfaceRequest(LSurface::Params *params) override;
    LSeat *createSeatRequest(LSeat::Params *params) override;
    LPointer *createPointerRequest(LPointer::Params *params) override;
    LKeyboard *createKeyboardRequest(LKeyboard::Params *params) override;
    LToplevelRole *createToplevelRoleRequest(LToplevelRole::Params *params) override;

    // Global scene used to render all outputs
    LScene scene;

    // Layer for views that are always at the bottom like wallpapers
    LLayerView backgroundLayer;

    // Layer where client windows are stacked
    LLayerView surfacesLayer;

    LLayerView workspacesLayer;

    // Layer for fullscreen toplevels
    LLayerView fullscreenLayer;

    // Layer for views that are always at the top like the dock, topbar or DND icons
    LLayerView overlayLayer;

    // Layer for tooltips and non client popups
    LLayerView tooltipsLayer;

    // Timer for updating the clock every minute
    LTimer *clockMinuteTimer;
    static Int32 millisecondsUntilNextMinute();

    // Shared texture used in all clock views
    LTexture *clockTexture = nullptr;

    // If true, we call scene->handlePointerEvent() once before scene->handlePaintGL().
    // The reason for this is that pointer events are only emitted when the pointer itself moves,
    // and they do not trigger when, for example, a view moves and positions itself under the cursor.
    // As a result, this call updates focus, cursor texture and so on...in those cases
    bool updatePointerBeforePaint = false;
};

#endif // COMPOSITOR_H
