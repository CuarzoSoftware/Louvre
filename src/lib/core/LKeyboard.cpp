#include <protocols/Wayland/private/RKeyboardPrivate.h>
#include <protocols/Wayland/GSeat.h>
#include <private/LSeatPrivate.h>
#include <private/LClientPrivate.h>
#include <private/LDataDevicePrivate.h>
#include <private/LKeyboardPrivate.h>
#include <LObject.h>
#include <LCompositor.h>
#include <LDNDManager.h>
#include <LClient.h>
#include <LCursor.h>
#include <LOutput.h>
#include <LTime.h>
#include <LLog.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

LKeyboard::LKeyboard(const void *params):
    LPRIVATE_INIT_UNIQUE(LKeyboard)
{
    L_UNUSED(params);

    seat()->imp()->keyboard = this;

    // Create XKB context
    imp()->xkbContext = xkb_context_new(XKB_CONTEXT_NO_FLAGS);

    // Set the default keymap
    setKeymap();
}

LKeyboard::~LKeyboard()
{
    if (imp()->xkbKeymapFd != -1)
    {
        close(imp()->xkbKeymapFd);
        imp()->xkbKeymapFd = -1;
    }

    if (imp()->xkbKeymapState)
    {
        xkb_state_unref(imp()->xkbKeymapState);
        imp()->xkbKeymapState = nullptr;
    }

    if (imp()->xkbKeymap)
    {
        xkb_keymap_unref(imp()->xkbKeymap);
        imp()->xkbKeymap = nullptr;
    }

    if (imp()->xkbContext)
    {
        xkb_context_unref(imp()->xkbContext);
        imp()->xkbContext = nullptr;
    }
}

void LKeyboard::setGrabbingSurface(LSurface *surface, Wayland::RKeyboard *keyboardResource)
{
    imp()->grabbingSurface = nullptr;
    imp()->grabbingKeyboardResource = nullptr;

    if (surface)
    {
        imp()->grabbingSurface = surface;
        imp()->grabbingKeyboardResource = keyboardResource;
    }
}

LSurface *LKeyboard::grabbingSurface() const
{
    return imp()->grabbingSurface;
}

RKeyboard *LKeyboard::grabbingKeyboardResource() const
{
    return imp()->grabbingKeyboardResource;
}

const LKeyboardModifiersEvent::Modifiers &LKeyboard::modifiers() const
{
    return imp()->currentModifiersState;
}

bool LKeyboard::setKeymap(const char *rules, const char *model, const char *layout, const char *variant, const char *options)
{
    const char *METHOD_NAME = "LKeyboard::setKeymap";

    if (imp()->xkbKeymapFd != -1)
    {
        close(imp()->xkbKeymapFd);
        imp()->xkbKeymapFd = -1;
    }

    if (imp()->xkbKeymapState)
    {
        xkb_state_unref(imp()->xkbKeymapState);
        imp()->xkbKeymapState = nullptr;
    }

    if (imp()->xkbKeymap)
    {
        xkb_keymap_unref(imp()->xkbKeymap);
        imp()->xkbKeymap = nullptr;
    }

    char *keymapString,*map;
    const char *xdgRuntimeDir;
    int res;

    imp()->xkbKeymapName.rules = rules;
    imp()->xkbKeymapName.model = model;
    imp()->xkbKeymapName.layout = layout;
    imp()->xkbKeymapName.variant = variant;
    imp()->xkbKeymapName.options = options;

    // Find a keymap matching suggestions
    imp()->xkbKeymap = xkb_keymap_new_from_names(imp()->xkbContext, &imp()->xkbKeymapName, XKB_KEYMAP_COMPILE_NO_FLAGS);

    if (!imp()->xkbKeymap)
    {
        LLog::error("[%s] Failed to set keymap with names Rules: %s, Model: %s, Layout: %s, Variant: %s, Opetions: %s. Restoring default keymap.",
                    METHOD_NAME, rules, model, layout, variant, options);

        goto fail;
    }

    // Get the keymap string
    keymapString = xkb_keymap_get_as_string(imp()->xkbKeymap, XKB_KEYMAP_FORMAT_TEXT_V1);

    // Store the keymap size
    imp()->xkbKeymapSize = strlen(keymapString) + 1;

    // Get the XDG_RUNTIME_DIR env
    xdgRuntimeDir = getenv("XDG_RUNTIME_DIR");

    if (!xdgRuntimeDir)
    {
        LLog::error("[%s] XDG_RUNTIME_DIR env not set. Using /tmp,", METHOD_NAME);
        xdgRuntimeDir = "/tmp";
    }

    // Open and store the file descritor
    imp()->xkbKeymapFd = open(xdgRuntimeDir, O_TMPFILE|O_RDWR|O_EXCL, 0600);

    if (imp()->xkbKeymapFd < 0)
    {
        LLog::error("[%s] Failed to allocate shared memory for keymap.", METHOD_NAME);
        goto fail;
    }

    // Write the keymap string
    res = ftruncate(imp()->xkbKeymapFd, imp()->xkbKeymapSize);
    L_UNUSED(res);
    map = (char*)mmap(NULL, imp()->xkbKeymapSize, PROT_READ|PROT_WRITE, MAP_SHARED, imp()->xkbKeymapFd, 0);
    memcpy(map, keymapString, imp()->xkbKeymapSize);
    munmap(map, imp()->xkbKeymapSize);

    // Keymap string not needed anymore
    free(keymapString);

    // Create a xkb keyboard state to handle modifiers
    imp()->xkbKeymapState = xkb_state_new(imp()->xkbKeymap);

    if (!imp()->xkbKeymapState)
    {
        LLog::error("[%s] Failed to get keymap state with names Rules: %s, Model: %s, Layout: %s, Variant: %s, Opetions: %s. Restoring default keymap.",
                    METHOD_NAME, rules, model, layout, variant, options);
        goto fail;
    }

    imp()->keymapFormat = WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1;

    for (auto client : compositor()->clients())
        for (auto gSeat : client->seatGlobals())
            for (auto rKeyboard : gSeat->keyboardResources())
                rKeyboard->keymap(keymapFormat(), keymapFd(), keymapSize());

    // Update LED idx
    imp()->leds[0] = xkb_keymap_led_get_index(imp()->xkbKeymap, XKB_LED_NAME_NUM);
    imp()->leds[1] = xkb_keymap_led_get_index(imp()->xkbKeymap, XKB_LED_NAME_CAPS);
    imp()->leds[2] = xkb_keymap_led_get_index(imp()->xkbKeymap, XKB_LED_NAME_SCROLL);

    return true;

    fail:

    if (rules != NULL || model != NULL || layout != NULL || variant != NULL || options != NULL)
    {
        if (setKeymap())
            return false;
        else
            LLog::error("[%s] Failed to set default keymap. Disabling keymap.", METHOD_NAME);
    }

    // Worst case, disables keymap

    imp()->xkbKeymapSize = 0;

    if (imp()->xkbKeymapFd != -1)
    {
        close(imp()->xkbKeymapFd);
        imp()->xkbKeymapFd = -1;
    }

    if (imp()->xkbKeymapState)
    {
        xkb_state_unref(imp()->xkbKeymapState);
        imp()->xkbKeymapState = nullptr;
    }

    if (imp()->xkbKeymap)
    {
        xkb_keymap_unref(imp()->xkbKeymap);
        imp()->xkbKeymap = nullptr;
    }

    imp()->xkbKeymapFd = open("/dev/null", O_RDONLY);

    imp()->keymapFormat = WL_KEYBOARD_KEYMAP_FORMAT_NO_KEYMAP;

    for (auto client : compositor()->clients())
        for (auto gSeat : client->seatGlobals())
            for (auto rKeyboard : gSeat->keyboardResources())
                rKeyboard->keymap(keymapFormat(), keymapFd(), keymapSize());

    return false;
}

Int32 LKeyboard::keymapFd() const
{
    return imp()->xkbKeymapFd;
}

Int32 LKeyboard::keymapSize() const
{
    return imp()->xkbKeymapSize;
}

UInt32 LKeyboard::keymapFormat() const
{
    return imp()->keymapFormat;
}

LSurface *LKeyboard::focus() const
{
    return imp()->keyboardFocusSurface;
}

void LKeyboard::setFocus(LSurface *surface)
{
    if (grabbingSurface())
        return;

    if (surface)
    {
        if (focus() == surface)
            return;

        if (focus())
        {
            LKeyboardLeaveEvent leaveEvent;

            for (auto gSeat : focus()->client()->seatGlobals())
                for (auto rKeyboard : gSeat->keyboardResources())
                    rKeyboard->leave(leaveEvent, focus()->surfaceResource());
        }

        if (!focus() || (focus() && focus()->client() != surface->client()))
            surface->client()->dataDevice().sendSelectionEvent();

        imp()->keyboardFocusSurface = nullptr;

        // Pack currently pressed keys
        wl_array keys;
        wl_array_init(&keys);

        for (UInt32 key : seat()->keyboard()->pressedKeys())
            *(UInt32*)wl_array_add(&keys, sizeof(UInt32)) = key;

        LKeyboardEnterEvent enterEvent;
        LKeyboardModifiersEvent modifiersEvent { modifiers() };

        for (auto gSeat : surface->client()->seatGlobals())
        {
            for (auto rKeyboard : gSeat->keyboardResources())
            {
                imp()->keyboardFocusSurface = surface;
                rKeyboard->enter(enterEvent, surface->surfaceResource(), &keys);
                rKeyboard->modifiers(modifiersEvent);
            }
        }

        wl_array_release(&keys);
    }
    else
    {
        // Remove focus from current surface
        if (focus())
        {
            LKeyboardLeaveEvent leaveEvent;

            for (auto gSeat : focus()->client()->seatGlobals())
                for (auto rKeyboard : gSeat->keyboardResources())
                    rKeyboard->leave(leaveEvent, focus()->surfaceResource());
        }

        imp()->keyboardFocusSurface = nullptr;
    }

    focusChanged();
}

void LKeyboard::sendKeyEvent(const LKeyboardKeyEvent &event)
{
    if (!focus())
        return;

    if (grabbingSurface())
    {
        grabbingKeyboardResource()->key(event);

        if (imp()->modifiersChanged)
            grabbingKeyboardResource()->modifiers(LKeyboardModifiersEvent(modifiers()));

        return;
    }

    const LKeyboardModifiersEvent modifiersEvent { modifiers() };

    for (auto gSeat : focus()->client()->seatGlobals())
    {
        for (auto rKeyboard : gSeat->keyboardResources())
        {
            rKeyboard->key(event);

            if (imp()->modifiersChanged)
                rKeyboard->modifiers(modifiersEvent);
        }
    }
}

xkb_keysym_t LKeyboard::keySymbol(UInt32 keyCode)
{
    if (!imp()->xkbKeymapState)
        return keyCode;

    return xkb_state_key_get_one_sym(imp()->xkbKeymapState, keyCode+8);
}

xkb_state *LKeyboard::keymapState() const
{
    return imp()->xkbKeymapState;
}

bool LKeyboard::isModActive(const char *name, xkb_state_component type) const
{
    if (!imp()->xkbKeymapState)
        return false;

    return xkb_state_mod_name_is_active(
                imp()->xkbKeymapState,
                name,
                type) == 1;
}

const std::vector<UInt32> &LKeyboard::pressedKeys() const
{
    return imp()->pressedKeys;
}

bool LKeyboard::isKeyCodePressed(UInt32 keyCode) const
{
    for (UInt32 key : imp()->pressedKeys)
        if (key == keyCode)
            return true;

    return false;
}

bool LKeyboard::isKeySymbolPressed(xkb_keysym_t keySymbol) const
{
    for (UInt32 key : imp()->pressedKeys)
        if (xkb_state_key_get_one_sym(imp()->xkbKeymapState, key + 8) == keySymbol)
            return true;

    return false;
}

// Since 4

Int32 LKeyboard::repeatRate() const
{
    return imp()->repeatRate;
}

Int32 LKeyboard::repeatDelay() const
{
    return imp()->repeatDelay;
}

void LKeyboard::setRepeatInfo(Int32 rate, Int32 msDelay)
{
    imp()->repeatRate = rate < 0 ? 0 : rate;
    imp()->repeatDelay = msDelay < 0 ? 0 : msDelay;

    for (auto client : compositor()->clients())
        for (auto gSeat : client->seatGlobals())
            for (auto rKeyboard : gSeat->keyboardResources())
                rKeyboard->repeatInfo(rate, msDelay);
}
