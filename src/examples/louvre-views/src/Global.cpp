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

static G::Cursors xCursors;
static G::Fonts _fonts;
static std::vector<App*>_apps;
static Tooltip *_tooltip;
static G::Textures _textures;
static G::ToplevelRegions _toplevelRegions;

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

void G::enableDocks(bool enabled)
{
    for (Output *o : outputs())
        if (o->dock)
            o->dock->setVisible(enabled);
}

void G::loadApps()
{
    int len;

    FILE *appsListFile = NULL;
    char *appsListPath = NULL;

    char appName[256];
    char appExec[256];
    char appIcon[256];

    const char *home = getenv("HOME");

    // Try loading user's apps.list first

    if (home)
    {
        appsListPath = joinPaths(home, "/.config/Louvre/apps.list");
        appsListFile = fopen(appsListPath, "r");
        free(appsListPath);
    }

    // If doesn't exist, fallback to default

    if (appsListFile == NULL)
    {
        appsListPath = joinPaths(ASSETS_PATH, "apps.list");
        appsListFile = fopen(appsListPath, "r");
        free(appsListPath);
    }

    // If doesn't exist, the dock is empty

    if (appsListFile == NULL)
    {
        LLog::error("[louvre-views] Failed to read apps.list.");
        return;
    }

    // Parse the apps.list file

    while (true)
    {
        // Read the app's name
        if (!fgets(appName, sizeof(appName), appsListFile))
            break;

        len = strlen(appName);

        if (len > 0)
            appName[len - 1] = '\0';
        else
            break;

        // Read the app's exec command
        if (!fgets(appExec, sizeof(appExec), appsListFile))
            break;

        len = strlen(appExec);

        if (len > 0)
            appExec[len - 1] = '\0';
        else
            break;

        // Read the app's icon path
        if (!fgets(appIcon, sizeof(appIcon), appsListFile))
            break;

        len = strlen(appIcon);

        if (len > 0)
            appIcon[len - 1] = '\0';
        else
            break;

        new App(appName, appExec, appIcon);
    }

    fclose(appsListFile);
}

std::vector<App *> &G::apps()
{
    return _apps;
}

void G::createTooltip()
{
    _tooltip = new Tooltip();
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

G::Textures *G::textures()
{
    return &_textures;
}

void G::loadTextures()
{
    LTexture *tmp = loadAssetsTexture("dock_app.png");

    if (tmp)
    {
        LTexture *hires = tmp->copyB(LSize(DOCK_ITEM_HEIGHT * 4));
        _textures.defaultAppIcon = hires->copyB(LSize(DOCK_ITEM_HEIGHT * 2));
        delete hires;
        delete tmp;
    }

    _textures.atlas = loadAssetsTexture("ui@2x.png");

    Float32 bufferScale = 2.f;
    LTexture *texture = _textures.atlas;
    TextureViewConf *conf;

    // Toplevel buttons
    for (UInt32 i = 0; i < 14; i++)
    {
        conf = &_textures.UIConf[i];
        conf->texture = texture;
        conf->customSrcRect = LRectF(2.f, 2.f + 14.f * Float32(i), 12.f, 12.f);
        conf->customDstSize = LSize(12, 12);
        conf->bufferScale = bufferScale;
    }

    conf = &_textures.UIConf[DecorationActiveTL];
    conf->texture = texture;
    conf->customSrcRect = LRectF(16.f, 59.5f, 70.f, 74.f);
    conf->customDstSize = LSize(70, 74);
    conf->bufferScale = bufferScale;

    conf = &_textures.UIConf[DecorationActiveTR];
    conf->texture = texture;
    conf->customSrcRect = LRectF(172.f - 16.f - 70.f, 59.5f, 70.f, 74.f);
    conf->customDstSize = LSize(70, 74);
    conf->bufferScale = bufferScale;
    conf->transform = LFramebuffer::Flipped;

    conf = &_textures.UIConf[DecorationActiveT];
    conf->texture = texture;
    conf->customSrcRect = LRectF(87.f, 59.5f, 1.f, 61.f);
    conf->customDstSize = LSize(1, 61);
    conf->bufferScale = bufferScale;

    conf = &_textures.UIConf[DecorationActiveL];
    conf->texture = texture;
    conf->customSrcRect = LRectF(16.f, 133.5f, 48.f, 0.5f);
    conf->customDstSize = LSize(48, 1);
    conf->bufferScale = bufferScale;

    conf = &_textures.UIConf[DecorationActiveR];
    conf->texture = texture;
    conf->customSrcRect = LRectF(172.f - 16.f - 48.f, 133.5f, 48.f, 0.1f);
    conf->customDstSize = LSize(48, 1);
    conf->bufferScale = bufferScale;
    conf->transform = LFramebuffer::Flipped;

    conf = &_textures.UIConf[DecorationActiveBL];
    conf->texture = texture;
    conf->customSrcRect = LRectF(16.f, 134.f, 82.f, 80.f);
    conf->customDstSize = LSize(82, 80);
    conf->bufferScale = bufferScale;

    conf = &_textures.UIConf[DecorationActiveBR];
    conf->texture = texture;
    conf->customSrcRect = LRectF(172.f - 16.f - 82.f, 134.f, 82.f, 80.f);
    conf->customDstSize = LSize(82, 80);
    conf->bufferScale = bufferScale;
    conf->transform = LFramebuffer::Flipped;

    conf = &_textures.UIConf[DecorationActiveB];
    conf->texture = texture;
    conf->customSrcRect = LRectF(98.f, 155.5f, 0.5f, 58.5f);
    conf->customDstSize = LSize(1, 59);
    conf->bufferScale = bufferScale;

    conf = &_textures.UIConf[DecorationInactiveTL];
    conf->texture = texture;
    conf->customSrcRect = LRectF(100.5f, 58.5f, 56.f, 63.f);
    conf->customDstSize = LSize(56, 63);
    conf->bufferScale = bufferScale;

    conf = &_textures.UIConf[DecorationInactiveTR];
    conf->texture = texture;
    conf->customSrcRect = LRectF(172.f - 100.5f - 56.f, 58.5f, 56.f, 63.f);
    conf->customDstSize = LSize(56, 63);
    conf->bufferScale = bufferScale;
    conf->transform = LFramebuffer::Flipped;

    conf = &_textures.UIConf[DecorationInactiveT];
    conf->texture = texture;
    conf->customSrcRect = LRectF(156.f, 58.5f, 0.5f, 56.f);
    conf->customDstSize = LSize(1, 56);
    conf->bufferScale = bufferScale;

    conf = &_textures.UIConf[DecorationInactiveBL];
    conf->texture = texture;
    conf->customSrcRect = LRectF(100.5f, 122.f, 71.f, 66.f);
    conf->customDstSize = LSize(71, 66);
    conf->bufferScale = bufferScale;

    conf = &_textures.UIConf[DecorationInactiveBR];
    conf->texture = texture;
    conf->customSrcRect = LRectF(172.f - 100.5f - 71.f, 122.f, 71.f, 66.f);
    conf->customDstSize = LSize(71, 66);
    conf->bufferScale = bufferScale;
    conf->transform = LFramebuffer::Flipped;

    conf = &_textures.UIConf[DecorationInactiveL];
    conf->texture = texture;
    conf->customSrcRect = LRectF(100.5f, 121.5f, 41.f, 0.5f);
    conf->customDstSize = LSize(41, 1);
    conf->bufferScale = bufferScale;

    conf = &_textures.UIConf[DecorationInactiveR];
    conf->texture = texture;
    conf->customSrcRect = LRectF(172.f - 100.5f - 41.f, 121.5f, 41.f, 0.5f);
    conf->customDstSize = LSize(41, 1);
    conf->bufferScale = bufferScale;
    conf->transform = LFramebuffer::Flipped;

    conf = &_textures.UIConf[DecorationInactiveB];
    conf->texture = texture;
    conf->customSrcRect = LRectF(171.5f, 136.f, 0.5f, 52.f);
    conf->customDstSize = LSize(1, 52);
    conf->bufferScale = bufferScale;

    conf = &_textures.UIConf[DecorationMaskBL];
    conf->texture = texture;
    conf->customSrcRect = LRectF(172.f - 11.f, 0.f, 11.f, 11.f);
    conf->customDstSize = LSize(11, 11);
    conf->bufferScale = bufferScale;
    conf->transform = LFramebuffer::Flipped180;

    conf = &_textures.UIConf[DecorationMaskBR];
    conf->texture = texture;
    conf->customSrcRect = LRectF(0.f, 0.f, 11.f, 11.f);
    conf->customDstSize = LSize(11, 11);
    conf->bufferScale = bufferScale;
    conf->transform = LFramebuffer::Rotated180;

    conf = &_textures.UIConf[TooltipT];
    conf->texture = texture;
    conf->customSrcRect = LRectF(0.f, 216.f - 0.5f - 21.f, 0.5f, 21.f);
    conf->customDstSize = LSize(1, 21);
    conf->bufferScale = bufferScale;
    conf->transform = LFramebuffer::Rotated180;

    conf = &_textures.UIConf[TooltipL];
    conf->texture = texture;
    conf->customSrcRect = LRectF(150.5f, 0.f, 21.f, 0.5f);
    conf->customDstSize = LSize(21, 1);
    conf->bufferScale = bufferScale;

    conf = &_textures.UIConf[TooltipR];
    conf->texture = texture;
    conf->customSrcRect = LRectF(0.5f, 0.f, 21.f, 0.5f);
    conf->customDstSize = LSize(21, 1);
    conf->bufferScale = bufferScale;
    conf->transform = LFramebuffer::Flipped;

    conf = &_textures.UIConf[TooltipB];
    conf->texture = texture;
    conf->customSrcRect = LRectF(172.f - 0.5f, 0.5f, 0.5f, 21.f);
    conf->customDstSize = LSize(1, 21);
    conf->bufferScale = bufferScale;

    conf = &_textures.UIConf[TooltipTL];
    conf->texture = texture;
    conf->customSrcRect = LRectF(216.f - 21.f - 0.5f, 150.5f, 21.f, 21.f);
    conf->customDstSize = LSize(21, 21);
    conf->bufferScale = bufferScale;
    conf->transform = LFramebuffer::Rotated90;

    conf = &_textures.UIConf[TooltipTR];
    conf->texture = texture;
    conf->customSrcRect = LRectF(0.5f, 172.f - 0.5f - 21.f, 21.f, 21.f);
    conf->customDstSize = LSize(21, 21);
    conf->bufferScale = bufferScale;
    conf->transform = LFramebuffer::Flipped90;

    conf = &_textures.UIConf[TooltipBR];
    conf->texture = texture;
    conf->customSrcRect = LRectF(0.5f, 0.5f, 21.f, 21.f);
    conf->customDstSize = LSize(21, 21);
    conf->bufferScale = bufferScale;
    conf->transform = LFramebuffer::Rotated270;

    conf = &_textures.UIConf[TooltipBL];
    conf->texture = texture;
    conf->customSrcRect = LRectF(150.5f, 0.5f, 21.f, 21.f);
    conf->customDstSize = LSize(21, 21);
    conf->bufferScale = bufferScale;

    conf = &_textures.UIConf[TooltipArrow];
    conf->texture = texture;
    conf->customSrcRect = LRectF(100.5f, 194.5f, 29.f, 17.f);
    conf->customDstSize = LSize(29, 17);
    conf->bufferScale = bufferScale;

    conf = &_textures.UIConf[DockL];
    conf->texture = texture;
    conf->customSrcRect = LRectF(216.f - 0.5f - 56.f, 16.f, 56.f, 133.f);
    conf->customDstSize = LSize(56, 133);
    conf->bufferScale = bufferScale;
    conf->transform = LFramebuffer::Rotated90;

    conf = &_textures.UIConf[DockC];
    conf->texture = texture;
    conf->customSrcRect = LRectF(216.f - 0.5f, 16.f, 0.5f, 133.f);
    conf->customDstSize = LSize(1, 133);
    conf->bufferScale = bufferScale;
    conf->transform = LFramebuffer::Rotated90;

    conf = &_textures.UIConf[DockR];
    conf->texture = texture;
    conf->customSrcRect = LRectF(0.5f, 23.f, 56.f, 133.f);
    conf->customDstSize = LSize(56, 133);
    conf->bufferScale = bufferScale;
    conf->transform = LFramebuffer::Rotated270;

    conf = &_textures.UIConf[DockDot];
    conf->texture = texture;
    conf->customSrcRect = LRectF(5.5f, 198.f, 5.f, 5.f);
    conf->customDstSize = LSize(5, 5);
    conf->bufferScale = bufferScale;

    conf = &_textures.UIConf[Logo];
    conf->texture = texture;
    conf->customSrcRect = LRectF(133.5f, 194.f, 20.f, 14.f);
    conf->customDstSize = LSize(20, 14);
    conf->bufferScale = bufferScale;
}

void G::setTexViewConf(LTextureView *view, UInt32 index)
{
    TextureViewConf *conf = &_textures.UIConf[index];
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

G::ToplevelRegions *G::toplevelRegions()
{
    return &_toplevelRegions;
}

void G::loadToplevelRegions()
{
    LRect activeTransRectsTL[] = TOPLEVEL_ACTIVE_TOP_LEFT_TRANS_REGION;

    for (UInt64 i = 0; i < sizeof(activeTransRectsTL)/sizeof(LRect); i++)
        _toplevelRegions.activeTransRegionTL.addRect(activeTransRectsTL[i]);

    LRect activeTransRectsTR[] = TOPLEVEL_ACTIVE_TOP_RIGHT_TRANS_REGION;

    for (UInt64 i = 0; i < sizeof(activeTransRectsTR)/sizeof(LRect); i++)
        _toplevelRegions.activeTransRegionTR.addRect(activeTransRectsTR[i]);

    LRect inactiveTransRectsTL[] = TOPLEVEL_INACTIVE_TOP_LEFT_TRANS_REGION;

    for (UInt64 i = 0; i < sizeof(inactiveTransRectsTL)/sizeof(LRect); i++)
        _toplevelRegions.inactiveTransRegionTL.addRect(inactiveTransRectsTL[i]);

    LRect inactiveTransRectsTR[] = TOPLEVEL_INACTIVE_TOP_RIGHT_TRANS_REGION;

    for (UInt64 i = 0; i < sizeof(inactiveTransRectsTR)/sizeof(LRect); i++)
        _toplevelRegions.inactiveTransRegionTR.addRect(inactiveTransRectsTR[i]);
}

void G::loadFonts()
{
    _fonts.regular = TextRenderer::loadFont("Inter");
    _fonts.semibold = TextRenderer::loadFont("Inter Semi Bold");

    if (_fonts.semibold)
        _textures.defaultTopbarAppName = G::font()->semibold->renderText("Louvre", 24);
}

G::Fonts *G::font()
{
    return &_fonts;
}

const char *G::transformName(LFramebuffer::Transform transform)
{
    switch (transform)
    {
    case LFramebuffer::Normal:
        return "No Transform";
    case LFramebuffer::Rotated90:
        return "Rotated 90°";
    case LFramebuffer::Rotated180:
        return "Rotated 180°";
    case LFramebuffer::Rotated270:
        return "Rotated 270°";
    case LFramebuffer::Flipped:
        return "Flipped";
    case LFramebuffer::Flipped90:
        return "Flipped & Rotated 90°";
    case LFramebuffer::Flipped180:
        return "Flipped & Rotated 180°";
    case LFramebuffer::Flipped270:
        return "Flipped & Rotated 270°";
    }

    return NULL;
}

LTexture *G::loadAssetsTexture(const char *name, bool exitOnFail)
{
    char *path = joinPaths(ASSETS_PATH, name);
    LTexture *tex = LOpenGL::loadTexture(path);
    free(path);

    if (exitOnFail && !tex)
    {
        LLog::fatal("[louvre-views] Failed to load texture %s.", name);
        exit(1);
    }

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
