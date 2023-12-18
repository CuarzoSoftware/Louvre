#include <LLog.h>
#include <string.h>
#include <LXCursor.h>
#include <LCursor.h>
#include <LOpenGL.h>
#include <stdio.h>

#include "Global.h"
#include "Compositor.h"
#include "Output.h"
#include "Dock.h"
#include "TextRenderer.h"
#include "App.h"
#include "Tooltip.h"
#include "Surface.h"

static G::DockTextures _dockTextures;
static G::ToplevelTextures _toplevelTextures;
static G::TooltipTextures _tooltipTextures;

static G::Cursors xCursors;
static G::Fonts _fonts;
static std::list<App*>_apps;

static Tooltip *_tooltip;

Compositor *G::compositor()
{
    return (Compositor*)LCompositor::compositor();
}

LScene *G::scene()
{
    return &compositor()->scene;
}

Pointer *G::pointer()
{
    return (Pointer*)compositor()->seat()->pointer();
}

std::list<Output *> &G::outputs()
{
    return (std::list<Output*>&)compositor()->outputs();
}

std::list<Surface *> &G::surfaces()
{
    return (std::list<Surface*>&)compositor()->surfaces();
}

void G::loadDockTextures()
{
    _dockTextures.left = loadAssetsTexture("dock_side.png");

    if (!_dockTextures.left)
    {
        LLog::fatal("[louvre-views] Failed to load dock_side.png texture.");
        exit(1);
    }

    _dockTextures.center = loadAssetsTexture("dock_clamp.png");

    if (!_dockTextures.center)
    {
        LLog::fatal("[louvre-views] Failed to load dock_center.png texture.");
        exit(1);
    }

    _dockTextures.right = _dockTextures.left->copyB(_dockTextures.left->sizeB(),
                                                    LRect(0,
                                                          0,
                                                          - _dockTextures.left->sizeB().w(),
                                                          _dockTextures.left->sizeB().h()));

    LTexture *tmp = loadAssetsTexture("dock_app.png");

    if (tmp)
    {
        LTexture *hires = tmp->copyB(LSize(DOCK_ITEM_HEIGHT * 4));
        _dockTextures.defaultApp = hires->copyB(LSize(DOCK_ITEM_HEIGHT * 2));
        delete hires;
        delete tmp;
    }

    if (!_dockTextures.defaultApp)
    {
        LLog::fatal("[louvre-views] Failed to load dock_app.png texture.");
        exit(1);
    }

    _dockTextures.dot = loadAssetsTexture("dock_app_dot.png");

    if (!_dockTextures.dot)
    {
        LLog::fatal("[louvre-views] Failed to load dock_dot.png texture.");
        exit(1);
    }
}

G::DockTextures &G::dockTextures()
{
    return _dockTextures;
}

void G::enableDocks(bool enabled)
{
    for (Output *o : outputs())
        if (o->dock)
            o->dock->setVisible(enabled);
}

void G::loadApps()
{
    FILE *file = NULL;

    char appList[256];
    char appName[256];
    char appExec[256];
    char appIcon[256];

    const char *home = getenv("HOME");

    if (!home)
        goto default_app;

    sprintf(appList, "%s/.config/Louvre/apps.list", home);

    file = fopen(appList, "r");

    default_app:
    if (file == NULL)
    {
        char *path = joinPaths(ASSETS_PATH, "apps.list");
        file = fopen(path, "r");
        free(path);
    }

    if (file == NULL)
        goto error;

    int len;

     // Buffer to hold each line
    while (true)
    {
        if (!fgets(appName, sizeof(appName), file))
            break;

        len = strlen(appName);

        if (len > 0)
            appName[len - 1] = '\0';
        else
            break;

        if (!fgets(appExec, sizeof(appExec), file))
            break;

        len = strlen(appExec);

        if (len > 0)
            appExec[len - 1] = '\0';
        else
            break;

        if (!fgets(appIcon, sizeof(appIcon), file))
            break;

        len = strlen(appIcon);

        if (len > 0)
            appIcon[len - 1] = '\0';
        else
            break;

        new App(appName, appExec, appIcon);
    }

    fclose(file);
    return;

    error:
    LLog::error("[louvre-views] Failed to read apps.list");

    if (file)
        fclose(file);
}

std::list<App *> &G::apps()
{
    return _apps;
}

void G::createTooltip()
{
    _tooltipTextures.decoration[TL] = loadAssetsTexture("container_top_left.png");
    _tooltipTextures.decoration[T]= loadAssetsTexture("container_clamp_top.png");
    _tooltipTextures.decoration[L] = loadAssetsTexture("container_clamp_side.png");
    _tooltipTextures.arrow = loadAssetsTexture("container_arrow.png");


    _tooltipTextures.decoration[TR] = _tooltipTextures.decoration[TL]->copyB(_tooltipTextures.decoration[TL]->sizeB(),
                                                                LRect(0,
                                                                      0,
                                                                      - _tooltipTextures.decoration[TL]->sizeB().w(),
                                                                      _tooltipTextures.decoration[TL]->sizeB().h()));

    _tooltipTextures.decoration[R] = _tooltipTextures.decoration[L]->copyB(_tooltipTextures.decoration[L]->sizeB(),
                                                                LRect(0,
                                                                      0,
                                                                      - _tooltipTextures.decoration[L]->sizeB().w(),
                                                                      _tooltipTextures.decoration[L]->sizeB().h()));

    _tooltipTextures.decoration[BR] = _tooltipTextures.decoration[TL]->copyB(_tooltipTextures.decoration[TL]->sizeB(),
                                                                LRect(0,
                                                                      0,
                                                                      - _tooltipTextures.decoration[TL]->sizeB().w(),
                                                                      - _tooltipTextures.decoration[TL]->sizeB().h()));

    _tooltipTextures.decoration[B] = _tooltipTextures.decoration[T]->copyB(_tooltipTextures.decoration[T]->sizeB(),
                                                          LRect(0,
                                                                0,
                                                                _tooltipTextures.decoration[T]->sizeB().w(),
                                                                - _tooltipTextures.decoration[T]->sizeB().h()));

    _tooltipTextures.decoration[BL] = _tooltipTextures.decoration[TL]->copyB(_tooltipTextures.decoration[TL]->sizeB(),
                                                                   LRect(0,
                                                                         0,
                                                                         _tooltipTextures.decoration[TL]->sizeB().w(),
                                                                         - _tooltipTextures.decoration[TL]->sizeB().h()));
    _tooltip = new Tooltip();
}

G::TooltipTextures &G::tooltipTextures()
{
    return _tooltipTextures;
}

Tooltip *G::tooltip()
{
    return _tooltip;
}

void G::loadCursors()
{
    xCursors.arrow = LXCursor::loadXCursorB("arrow");

    if (xCursors.arrow)
        LCompositor::compositor()->cursor()->replaceDefaultB(xCursors.arrow->texture(), xCursors.arrow->hotspotB());

    xCursors.hand2 = LXCursor::loadXCursorB("hand2");
    xCursors.top_left_corner = LXCursor::loadXCursorB("top_left_corner");
    xCursors.top_right_corner= LXCursor::loadXCursorB("top_right_corner");
    xCursors.bottom_left_corner = LXCursor::loadXCursorB("bottom_left_corner");
    xCursors.bottom_right_corner = LXCursor::loadXCursorB("bottom_right_corner");
    xCursors.left_side = LXCursor::loadXCursorB("left_side");
    xCursors.top_side = LXCursor::loadXCursorB("top_side");
    xCursors.right_side = LXCursor::loadXCursorB("right_side");
    xCursors.bottom_side = LXCursor::loadXCursorB("bottom_side");
}

G::Cursors &G::cursors()
{
    return xCursors;
}

void G::loadToplevelTextures()
{
    _toplevelTextures.activeTL = loadAssetsTexture("toplevel_active_top_left.png");
    _toplevelTextures.activeT = loadAssetsTexture("toplevel_active_top_clamp.png");
    _toplevelTextures.activeTR = _toplevelTextures.activeTL->copyB(_toplevelTextures.activeTL->sizeB(),
                                                                  LRect(0,
                                                                        0,
                                                                        -_toplevelTextures.activeTL->sizeB().w(),
                                                                        _toplevelTextures.activeTL->sizeB().h()));
    _toplevelTextures.activeL = loadAssetsTexture("toplevel_active_side_clamp.png");
    _toplevelTextures.activeR = _toplevelTextures.activeL->copyB(_toplevelTextures.activeL->sizeB(),
                                                                  LRect(0,
                                                                        0,
                                                                        -_toplevelTextures.activeL->sizeB().w(),
                                                                        _toplevelTextures.activeL->sizeB().h()));
    _toplevelTextures.activeBL = loadAssetsTexture("toplevel_active_bottom_left.png");
    _toplevelTextures.activeB = loadAssetsTexture("toplevel_active_bottom_clamp.png");
    _toplevelTextures.activeBR = _toplevelTextures.activeBL->copyB(_toplevelTextures.activeBL->sizeB(),
                                                                  LRect(0,
                                                                        0,
                                                                        -_toplevelTextures.activeBL->sizeB().w(),
                                                                        _toplevelTextures.activeBL->sizeB().h()));

    _toplevelTextures.inactiveTL = loadAssetsTexture("toplevel_inactive_top_left.png");
    _toplevelTextures.inactiveT = loadAssetsTexture("toplevel_inactive_top_clamp.png");
    _toplevelTextures.inactiveTR = _toplevelTextures.inactiveTL->copyB(_toplevelTextures.inactiveTL->sizeB(),
                                                                  LRect(0,
                                                                        0,
                                                                        -_toplevelTextures.inactiveTL->sizeB().w(),
                                                                        _toplevelTextures.inactiveTL->sizeB().h()));
    _toplevelTextures.inactiveL = loadAssetsTexture("toplevel_inactive_side_clamp.png");
    _toplevelTextures.inactiveR = _toplevelTextures.inactiveL->copyB(_toplevelTextures.inactiveL->sizeB(),
                                                                  LRect(0,
                                                                        0,
                                                                        -_toplevelTextures.inactiveL->sizeB().w(),
                                                                        _toplevelTextures.inactiveL->sizeB().h()));
    _toplevelTextures.inactiveBL = loadAssetsTexture("toplevel_inactive_bottom_left.png");
    _toplevelTextures.inactiveB = loadAssetsTexture("toplevel_inactive_bottom_clamp.png");
    _toplevelTextures.inactiveBR = _toplevelTextures.inactiveBL->copyB(_toplevelTextures.inactiveBL->sizeB(),
                                                                  LRect(0,
                                                                        0,
                                                                        -_toplevelTextures.inactiveBL->sizeB().w(),
                                                                        _toplevelTextures.inactiveBL->sizeB().h()));


    _toplevelTextures.maskBL = loadAssetsTexture("toplevel_border_radius_mask.png");
    _toplevelTextures.maskBR = _toplevelTextures.maskBL->copyB(_toplevelTextures.maskBL->sizeB(),
                                                                  LRect(0,
                                                                        0,
                                                                        -_toplevelTextures.maskBL->sizeB().w(),
                                                                        _toplevelTextures.maskBL->sizeB().h()));

    LRect activeTransRectsTL[] = TOPLEVEL_ACTIVE_TOP_LEFT_TRANS_REGION;

    for (UInt64 i = 0; i < sizeof(activeTransRectsTL)/sizeof(LRect); i++)
        _toplevelTextures.activeTransRegionTL.addRect(activeTransRectsTL[i]);

    LRect activeTransRectsTR[] = TOPLEVEL_ACTIVE_TOP_RIGHT_TRANS_REGION;

    for (UInt64 i = 0; i < sizeof(activeTransRectsTR)/sizeof(LRect); i++)
        _toplevelTextures.activeTransRegionTR.addRect(activeTransRectsTR[i]);


    LRect inactiveTransRectsTL[] = TOPLEVEL_INACTIVE_TOP_LEFT_TRANS_REGION;

    for (UInt64 i = 0; i < sizeof(inactiveTransRectsTL)/sizeof(LRect); i++)
        _toplevelTextures.inactiveTransRegionTL.addRect(inactiveTransRectsTL[i]);

    LRect inactiveTransRectsTR[] = TOPLEVEL_INACTIVE_TOP_RIGHT_TRANS_REGION;

    for (UInt64 i = 0; i < sizeof(inactiveTransRectsTR)/sizeof(LRect); i++)
        _toplevelTextures.inactiveTransRegionTR.addRect(inactiveTransRectsTR[i]);

    _toplevelTextures.inactiveButton = loadAssetsTexture("button_inactive.png");
    _toplevelTextures.activeCloseButton = loadAssetsTexture("button_close.png");
    _toplevelTextures.activeCloseButtonHover = loadAssetsTexture("button_close_hover.png");
    _toplevelTextures.activeCloseButtonPressed = loadAssetsTexture("button_close_pressed.png");
    _toplevelTextures.activeMinimizeButton = loadAssetsTexture("button_minimize.png");
    _toplevelTextures.activeMinimizeButtonHover = loadAssetsTexture("button_minimize_hover.png");
    _toplevelTextures.activeMinimizeButtonPressed = loadAssetsTexture("button_minimize_pressed.png");
    _toplevelTextures.activeMaximizeButton = loadAssetsTexture("button_maximize.png");
    _toplevelTextures.activeMaximizeButtonHover = loadAssetsTexture("button_maximize_hover.png");
    _toplevelTextures.activeMaximizeButtonPressed = loadAssetsTexture("button_maximize_pressed.png");
    _toplevelTextures.activeFullscreenButtonHover = loadAssetsTexture("button_fullscreen_hover.png");
    _toplevelTextures.activeFullscreenButtonPressed = loadAssetsTexture("button_fullscreen_pressed.png");
    _toplevelTextures.activeUnfullscreenButtonHover = loadAssetsTexture("button_unfullscreen_hover.png");
    _toplevelTextures.activeUnfullscreenButtonPressed = loadAssetsTexture("button_unfullscreen_pressed.png");

    _toplevelTextures.logo = loadAssetsTexture("logo.png");
}

G::ToplevelTextures &G::toplevelTextures()
{
    return _toplevelTextures;
}

void G::loadFonts()
{
    _fonts.regular = TextRenderer::loadFont("Inter");
    _fonts.semibold = TextRenderer::loadFont("Inter Semi Bold");

    if (_fonts.semibold)
        G::toplevelTextures().defaultTopbarAppName = G::font()->semibold->renderText("Louvre", 24);
}

G::Fonts *G::font()
{
    return &_fonts;
}

LTexture *G::loadAssetsTexture(const char *name)
{
    char *path = joinPaths(ASSETS_PATH, name);
    LTexture *tex = LOpenGL::loadTexture(path);
    free(path);
    return tex;
}

char *G::joinPaths(const char *path1, const char *path2)
{
    size_t len1 = strlen(path1);
    size_t len2 = strlen(path2);

    char *result = (char *)malloc(len1 + len2 + 2);

    // Copy the first path
    snprintf(result, len1 + 1, "%s", path1);

    // Add a '/' if needed
    if (result[len1 - 1] != '/' && path2[0] != '/')
    {
        snprintf(result + len1, 2, "/");
        len1++;
    }

    // Concatenate the second path
    snprintf(result + len1, len2 + 1, "%s", path2);

    return result;
}

void G::enableParentScalingChildren(LView *parent, bool enabled)
{
    for (LView *child : parent->children())
    {
        child->enableParentScaling(enabled);
        enableParentScalingChildren(child, enabled);
    }
}

void G::enableClippingChildren(LView *parent, bool enabled)
{
    for (LView *child : parent->children())
    {
        child->enableClipping(enabled);
        enableClippingChildren(child, enabled);
    }
}

Output *G::mostIntersectedOuput(LView *view)
{
    LBox box = view->boundingBox();
    LRect rect(box.x1, box.y1, box.x2 - box.x1, box.y2 - box.y1);
    Output *bestOutput = nullptr;
    Int32 bestArea = 0;
    LBox extents;

    for (Output *o : outputs())
    {
        LRegion reg;
        reg.addRect(rect);
        reg.clip(o->rect());
        extents = reg.extents();
        Int32 area = (extents.x2 - extents.x1) * (extents.y2 - extents.y1);

        if (area > bestArea)
        {
            bestArea = area;
            bestOutput = o;
        }
    }

    return bestOutput;
}

void G::reparentWithSubsurfaces(Surface *surf, LView *newParent, bool onlySubsurfaces)
{
    surf->getView()->setParent(newParent);

    for (Surface *s : surfaces())
    {
        if (s->parent() == surf && s->roleId() != LSurface::Undefined && !s->cursorRole())
        {
            if ((onlySubsurfaces && s->subsurface()) || !onlySubsurfaces || s->popup())
                G::reparentWithSubsurfaces(s, newParent, onlySubsurfaces);
        }
    }
}

void G::arrangeOutputs()
{
    Int32 x = 0;

    for (Output *o : outputs())
    {
        o->setPos(LPoint(x, 0));
        x += o->size().w();
        o->repaint();
    }
}

Toplevel *G::searchFullscreenParent(Surface *parent)
{
    if (!parent)
        return nullptr;

    if (parent->toplevel() && parent->toplevel()->fullscreen())
    {
        Toplevel *tl = (Toplevel*)parent->toplevel();
        return tl;
    }

    return searchFullscreenParent((Surface*)parent->parent());
}

void G::repositionNonVisibleToplevelChildren(Output *target, Surface *toplevel)
{
    for (Surface *s : (std::list<Surface*>&)toplevel->children())
    {
        if (!s->toplevel())
            continue;

        LBox box = s->getView()->boundingBox();

        if (!LRect(box.x1, box.y1, box.x2 - box.x1, box.y2 - box.y1).intersects(target->rect()))
            s->setPos(target->pos().x() + 200 + rand() % 200,
                      target->pos().y() + 200 + rand() % 200);

        G::repositionNonVisibleToplevelChildren(target, s);
    }
}
