#include "Dock.h"
#include "Global.h"
#include "Compositor.h"
#include "Output.h"
#include "DockItem.h"
#include "Pointer.h"
#include "App.h"
#include "DockApp.h"
#include "src/Tooltip.h"

#include <LSessionLockManager.h>
#include <LTextureView.h>
#include <LCursor.h>
#include <LLog.h>

// Add it to the overlayLayer so it is always on top
Dock::Dock(Output *output) :
    LLayerView(&G::compositor()->overlayLayer)
{
    separator.setUserData(DockSeparatorType);
    this->output = output;
    output->dock = this;

    // Detach it from the overlay layer just in case we want to move in the future
    enableParentOffset(false);

    // Enable input so it shows/hides when the cursor is over
    enablePointerEvents(true);

    // Allow views behind to get pointer events
    enableBlockPointer(true);

    separator.setSize(1, DOCK_ITEM_HEIGHT);

    // Create app items
    for (App *app : G::apps())
        new DockApp(app, this);

    // Clone items from another already existing dock
    for (Output *o : G::outputs())
    {
        if (o != output && o->dock)
        {
            for (LView *v : o->dock->itemsContainer.children())
            {
                DockItem *item = (DockItem*)v;
                DockItem *itemClone = new DockItem(item->surface, this);
                itemClone->enableScaling(false);
            }
            break;
        }
    }

    update();
}

Dock::~Dock()
{
    alive = false;
    anim.stop();

    while (!itemsContainer.children().empty())
        delete itemsContainer.children().back();

    while (!appsContainer.children().empty())
    {
        if (appsContainer.children().back() == &separator)
            separator.setParent(nullptr);

        delete appsContainer.children().back();
    }

    output->dock = nullptr;
}

void Dock::update()
{
    if (!alive)
        return;

    Int32 dockWidth { DOCK_PADDING };

    if (itemsContainer.children().empty())
        separator.setParent(nullptr);
    else
    {
        separator.setParent(nullptr);
        separator.setParent(&appsContainer);
    }

    for (LView *item : appsContainer.children())
    {
        if (item->userData() == DockSeparatorType)
        {
            static_cast<LSolidColorView*>(item)->setPos(dockWidth, DOCK_PADDING + (DOCK_ITEM_HEIGHT - item->size().h())/2);
            dockWidth += DOCK_SPACING;
        }
        else if(item->userData() == DockAppType)
        {
            DockApp &dockApp { static_cast<DockApp&>(*item) };
            dockApp.setPos(dockWidth, - 2 - dockApp.app->dockAppsAnimationOffset.y() + DOCK_PADDING + (DOCK_ITEM_HEIGHT - item->size().h())/2);
            dockApp.dot.setVisible(dockApp.app->state == App::Running && dockApp.app->dockAppsAnimationOffset.y() == 0);

            if (pointerIsOver() && G::tooltip()->targetView == item)
                G::tooltip()->show(item->pos().x() + item->size().w() / 2, item->pos().y());
        }

        dockWidth += item->size().w();

        if (item != appsContainer.children().back())
            dockWidth += DOCK_SPACING;
    }

    for (LView *it : itemsContainer.children())
    {
        LTextureView &item { static_cast<LTextureView&>(*it) };

        item.setPos(dockWidth, DOCK_PADDING + (DOCK_ITEM_HEIGHT - item.size().h())/2);
        dockWidth += item.size().w();

        if (it != itemsContainer.children().back())
            dockWidth += DOCK_SPACING;

        if (pointerIsOver() && G::tooltip()->targetView == it)
            G::tooltip()->show(item.pos().x() + item.size().w() / 2, item.pos().y() - 8);
    }

    dockWidth += DOCK_PADDING;

    setSize(output->rect().w(),
            DOCK_HEIGHT + 6);

    setPos(output->rect().x() + (output->rect().w() - size().w()) / 2,
           output->rect().h() - (size().h() - 1) * visiblePercent);

    dockLeft.setPos(0, 0);
    dockCenter.setPos(dockLeft.nativePos().x() + dockLeft.size().w(), 0);
    dockCenter.setDstSize(dockWidth - 2 * DOCK_BORDER_RADIUS, dockCenter.nativeSize().h());
    dockRight.setPos(dockCenter.nativePos().x() + dockCenter.size().w(), 0);
    dockContainer.setSize(dockLeft.size().w() + dockCenter.size().w() + dockRight.size().w(), DOCK_HEIGHT);
    dockContainer.setPos((size().w() - dockContainer.size().w()) / 2, - DOCK_SHADOW_SIZE);
    appsContainer.setPos(DOCK_SHADOW_SIZE, DOCK_SHADOW_SIZE);
    itemsContainer.setPos(DOCK_SHADOW_SIZE, DOCK_SHADOW_SIZE);
    output->repaint();
}

void Dock::show()
{
    if (anim.running() || visiblePercent != 0.f)
        return;

    dockContainer.setVisible(true);
    dockContainer.setParent(this);

    anim.setDuration(250);

    anim.setOnUpdateCallback(
    [this](LAnimation *anim)
    {
        visiblePercent = 1.f - powf(1.f - anim->value(), 2.f);
        update();
        output->repaint();
    });

    anim.setOnFinishCallback(
    [this](LAnimation *)
    {
        if (!pointerIsOver())
        {
            hide();
            return;
        }

        LRegion input;
        input.addRect(0, size().h()/2, size().w(), DOCK_HEIGHT + 32);
        input.addRect(dockContainer.nativePos().x() + DOCK_SHADOW_SIZE / 2,
                      0,
                      dockContainer.size().w() - DOCK_SHADOW_SIZE,
                      DOCK_HEIGHT + 32);
        setInputRegion(&input);
    });

    anim.start();
}

void Dock::hide()
{
    if (anim.running() || visiblePercent != 1.f)
        return;

    anim.setDuration(250);

    anim.setOnUpdateCallback(
    [this](LAnimation *anim)
    {
        visiblePercent = 1.f - powf(anim->value(), 2.f);
        update();
        output->repaint();
    });

    anim.setOnFinishCallback(
    [this](LAnimation *)
    {
        dockContainer.setVisible(false);
        dockContainer.setParent(nullptr);
        G::tooltip()->hide();
        G::tooltip()->targetView = nullptr;
        setInputRegion(nullptr);
    });

    anim.start();
}

void Dock::pointerEnterEvent(const LPointerEnterEvent &)
{
    showResistanceCount = 0;
}

void Dock::pointerMoveEvent(const LPointerMoveEvent &)
{
    if (visiblePercent == 1.f && !G::pointer()->cursorOwner)
    {
        cursor()->useDefault();
        update();
    }

    if (showResistanceCount > showResistance)
        show();
    else
        showResistanceCount++;
}

void Dock::pointerLeaveEvent(const LPointerLeaveEvent &)
{
    G::tooltip()->hide();
    showResistanceCount = 0;
    hide();
}

bool Dock::nativeMapped() const noexcept
{
    return compositor()->sessionLockManager()->state() == LSessionLockManager::Unlocked;
}
