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

LKeyboard::LKeyboard(Params *params):
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

    for (LClient *c : compositor()->clients())
        for (Protocols::Wayland::GSeat *s : c->seatGlobals())
            if (s->keyboardResource())
                s->keyboardResource()->keymap(keymapFormat(), keymapFd(), keymapSize());

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

    for (LClient *c : compositor()->clients())
        for (Protocols::Wayland::GSeat *s : c->seatGlobals())
            if (s->keyboardResource())
                s->keyboardResource()->keymap(keymapFormat(), keymapFd(), keymapSize());

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

const LKeyboard::KeyboardModifiersState &LKeyboard::modifiersState() const
{
    return imp()->modifiersState;
}

void LKeyboard::setFocus(LSurface *surface)
{
    if (grabbingSurface())
        return;

    if (surface)
    {
        // If already has focus
        if (focus() == surface)
            return;
        else
        {
            // If another surface has focus
            if (focus())
            {
                UInt32 serial = LTime::nextSerial();

                for (Wayland::GSeat *s : focus()->client()->seatGlobals())
                {
                    if (s->keyboardResource())
                    {
                        s->keyboardResource()->imp()->serials.leave = serial;
                        s->keyboardResource()->leave(serial, focus()->surfaceResource());
                        break;
                    }
                }
            }

            if (!focus() || (focus() && focus()->client() != surface->client()))
                surface->client()->dataDevice().sendSelectionEvent();

            // If the new surface has no wl_pointer then it is like calling setFocus(nullptr)
            imp()->keyboardFocusSurface = nullptr;

            UInt32 serial = LTime::nextSerial();

            // Pack currently pressed keys
            wl_array keys;
            wl_array_init(&keys);

            for (UInt32 key : seat()->keyboard()->pressedKeys())
            {
                UInt32 *p = (UInt32*)wl_array_add(&keys, sizeof(UInt32));
                *p = key;
            }

            for (Wayland::GSeat *s : surface->client()->seatGlobals())
            {
                if (s->keyboardResource())
                {
                    imp()->keyboardFocusSurface = surface;
                    s->keyboardResource()->imp()->serials.enter = serial;
                    s->keyboardResource()->enter(serial, surface->surfaceResource(), &keys);
                    s->keyboardResource()->imp()->serials.modifiers = serial;
                    s->keyboardResource()->modifiers(
                        serial,
                        modifiersState().depressed,
                        modifiersState().latched,
                        modifiersState().locked,
                        modifiersState().group);
                }
            }

            wl_array_release(&keys);
        }
    }
    else
    {
        // Remove focus from current surface
        if (focus())
        {
            UInt32 serial = LTime::nextSerial();
            for (Wayland::GSeat *s : focus()->client()->seatGlobals())
            {
                if (s->keyboardResource())
                {
                    s->keyboardResource()->imp()->serials.leave = serial;
                    s->keyboardResource()->leave(serial, focus()->surfaceResource());
                }
            }
        }
        imp()->keyboardFocusSurface = nullptr;
    }

    focusChanged();
}

void LKeyboard::sendKeyEvent(UInt32 keyCode, KeyState keyState)
{
    // If no surface has focus
    if (!focus())
        return;

    UInt32 serial = LTime::nextSerial();
    UInt32 ms = LTime::ms();

    if (grabbingSurface())
    {
        grabbingKeyboardResource()->imp()->serials.key = serial;
        grabbingKeyboardResource()->key(serial, ms, keyCode, keyState);
        return;
    }

    for (Wayland::GSeat *s : focus()->client()->seatGlobals())
    {
        if (s->keyboardResource())
        {
            s->keyboardResource()->imp()->serials.key = serial;
            s->keyboardResource()->key(serial, ms, keyCode, keyState);
        }
    }
}

void LKeyboard::sendModifiersEvent(UInt32 depressed, UInt32 latched, UInt32 locked, UInt32 group)
{
    if (!focus())
        return;

    UInt32 serial = LTime::nextSerial();

    if (grabbingSurface())
    {
        grabbingKeyboardResource()->imp()->serials.modifiers = serial;
        grabbingKeyboardResource()->modifiers(serial, depressed, latched, locked, group);
        return;
    }

    for (Wayland::GSeat *s : focus()->client()->seatGlobals())
    {
        if (s->keyboardResource())
        {
            s->keyboardResource()->imp()->serials.modifiers = serial;
            s->keyboardResource()->modifiers(serial, depressed, latched, locked, group);
        }
    }
}

void LKeyboard::sendModifiersEvent()
{
    sendModifiersEvent(imp()->modifiersState.depressed, imp()->modifiersState.latched, imp()->modifiersState.locked, imp()->modifiersState.group);
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

bool LKeyboard::isModActive(const char *name) const
{
    if (!imp()->xkbKeymapState)
        return false;

    return xkb_state_mod_name_is_active(
                imp()->xkbKeymapState,
                name,
                XKB_STATE_MODS_EFFECTIVE);
}

const std::vector<UInt32> &LKeyboard::pressedKeys() const
{
    return imp()->pressedKeys;
}

bool LKeyboard::isKeyCodePressed(UInt32 keyCode) const
{
    for (UInt32 key : imp()->pressedKeys)
    {
        if (key == keyCode)
            return true;
    }
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
    if (rate < 0)
        imp()->repeatRate = 0;
    else
        imp()->repeatRate = rate;

    if (msDelay < 0)
        imp()->repeatDelay = 0;
    else
        imp()->repeatDelay = msDelay;

    for (LClient *c : compositor()->clients())
    {
        for (Protocols::Wayland::GSeat *s : c->seatGlobals())
            if (s->keyboardResource())
                s->keyboardResource()->repeatInfo(rate, msDelay);
    }
}
