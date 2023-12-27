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

static G::TextureViewConf texViewConfs[34];

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
    _toplevelTextures.atlas = LOpenGL::loadTexture("/home/eduardo/Arduino/atlas@2x.png");

    Float32 bufferScale = 2.f;
    TextureViewConf *conf;

    for (UInt32 i = 0; i < 14; i++)
    {
        conf = &texViewConfs[i];
        conf->texture = _toplevelTextures.atlas;
        conf->customSrcRect = LRectF(2.f, 2.f + 14.f * Float32(i), 12.f, 12.f);
        conf->customDstSize = LSize(12, 12);
        conf->bufferScale = bufferScale;
    }

    conf = &texViewConfs[DecorationActiveTL];
    conf->texture = _toplevelTextures.atlas;
    conf->customSrcRect = LRectF(16.f, 59.5f, 70.f, 74.f);
    conf->customDstSize = LSize(70, 74);
    conf->bufferScale = bufferScale;

    conf = &texViewConfs[DecorationActiveTR];
    conf->texture = _toplevelTextures.atlas;
    conf->customSrcRect = LRectF(172.f - 16.f - 70.f, 59.5f, 70.f, 74.f);
    conf->customDstSize = LSize(70, 74);
    conf->bufferScale = bufferScale;
    conf->transform = LFramebuffer::Flipped;

    conf = &texViewConfs[DecorationActiveT];
    conf->texture = _toplevelTextures.atlas;
    conf->customSrcRect = LRectF(87.f, 59.5f, 1.f, 61.f);
    conf->customDstSize = LSize(1, 61);
    conf->bufferScale = bufferScale;

    conf = &texViewConfs[DecorationActiveL];
    conf->texture = _toplevelTextures.atlas;
    conf->customSrcRect = LRectF(16.f, 133.5f, 48.f, 0.5f);
    conf->customDstSize = LSize(48, 1);
    conf->bufferScale = bufferScale;

    conf = &texViewConfs[DecorationActiveR];
    conf->texture = _toplevelTextures.atlas;
    conf->customSrcRect = LRectF(172.f - 16.f - 48.f, 133.5f, 48.f, 0.5f);
    conf->customDstSize = LSize(48, 1);
    conf->bufferScale = bufferScale;
    conf->transform = LFramebuffer::Flipped;

    conf = &texViewConfs[DecorationActiveBL];
    conf->texture = _toplevelTextures.atlas;
    conf->customSrcRect = LRectF(16.f, 134.f, 82.f, 80.f);
    conf->customDstSize = LSize(82, 80);
    conf->bufferScale = bufferScale;

    conf = &texViewConfs[DecorationActiveBR];
    conf->texture = _toplevelTextures.atlas;
    conf->customSrcRect = LRectF(172.f - 16.f - 82.f, 134.f, 82.f, 80.f);
    conf->customDstSize = LSize(82, 80);
    conf->bufferScale = bufferScale;
    conf->transform = LFramebuffer::Flipped;

    conf = &texViewConfs[DecorationActiveB];
    conf->texture = _toplevelTextures.atlas;
    conf->customSrcRect = LRectF(98.f, 155.5f, 0.5f, 58.5f);
    conf->customDstSize = LSize(1, 59);
    conf->bufferScale = bufferScale;

    conf = &texViewConfs[DecorationInactiveTL];
    conf->texture = _toplevelTextures.atlas;
    conf->customSrcRect = LRectF(100.5f, 58.5f, 56.f, 63.f);
    conf->customDstSize = LSize(56, 63);
    conf->bufferScale = bufferScale;

    conf = &texViewConfs[DecorationInactiveTR];
    conf->texture = _toplevelTextures.atlas;
    conf->customSrcRect = LRectF(172.f - 100.5f - 56.f, 58.5f, 56.f, 63.f);
    conf->customDstSize = LSize(56, 63);
    conf->bufferScale = bufferScale;
    conf->transform = LFramebuffer::Flipped;

    conf = &texViewConfs[DecorationInactiveT];
    conf->texture = _toplevelTextures.atlas;
    conf->customSrcRect = LRectF(156.f, 58.5f, 0.5f, 56.f);
    conf->customDstSize = LSize(1, 56);
    conf->bufferScale = bufferScale;

    conf = &texViewConfs[DecorationInactiveBL];
    conf->texture = _toplevelTextures.atlas;
    conf->customSrcRect = LRectF(100.5f, 122.f, 71.f, 66.f);
    conf->customDstSize = LSize(71, 66);
    conf->bufferScale = bufferScale;

    conf = &texViewConfs[DecorationInactiveBR];
    conf->texture = _toplevelTextures.atlas;
    conf->customSrcRect = LRectF(172.f - 100.5f - 71.f, 122.f, 71.f, 66.f);
    conf->customDstSize = LSize(71, 66);
    conf->bufferScale = bufferScale;
    conf->transform = LFramebuffer::Flipped;

    conf = &texViewConfs[DecorationInactiveL];
    conf->texture = _toplevelTextures.atlas;
    conf->customSrcRect = LRectF(100.5f, 121.5f, 41.f, 0.5f);
    conf->customDstSize = LSize(41, 1);
    conf->bufferScale = bufferScale;

    conf = &texViewConfs[DecorationInactiveR];
    conf->texture = _toplevelTextures.atlas;
    conf->customSrcRect = LRectF(172.f - 100.5f - 41.f, 121.5f, 41.f, 0.5f);
    conf->customDstSize = LSize(41, 1);
    conf->bufferScale = bufferScale;
    conf->transform = LFramebuffer::Flipped;

    conf = &texViewConfs[DecorationInactiveB];
    conf->texture = _toplevelTextures.atlas;
    conf->customSrcRect = LRectF(171.5f, 136.f, 0.5f, 52.f);
    conf->customDstSize = LSize(1, 52);
    conf->bufferScale = bufferScale;

    conf = &texViewConfs[DecorationMaskBL];
    conf->texture = _toplevelTextures.atlas;
    conf->customSrcRect = LRectF(172.f - 11.f, 0.f, 11.f, 11.f);
    conf->customDstSize = LSize(11, 11);
    conf->bufferScale = bufferScale;
    conf->transform = LFramebuffer::Flipped180;

    conf = &texViewConfs[DecorationMaskBR];
    conf->texture = _toplevelTextures.atlas;
    conf->customSrcRect = LRectF(0.f, 0.f, 11.f, 11.f);
    conf->customDstSize = LSize(11, 11);
    conf->bufferScale = bufferScale;
    conf->transform = LFramebuffer::Rotated180;


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

    _toplevelTextures.logo = loadAssetsTexture("logo.png");
}

G::ToplevelTextures &G::toplevelTextures()
{
    return _toplevelTextures;
}

void G::setTexViewConf(LTextureView *view, TextureConfIndex index)
{
    TextureViewConf *conf = &texViewConfs[index];
    view->setTexture(conf->texture);
    view->enableCustomColor(conf->enableCustomColor);
    view->setCustomColor(conf->customColor);
    view->enableDstSize(conf->enableCustomDstSize);
    view->setDstSize(conf->customDstSize);
    view->enableSrcRect(conf->enableCustomSrcRect);
    view->setSrcRect(conf->customSrcRect);
    view->setBufferScale(conf->bufferScale);
    view->setTransform(conf->transform);
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
