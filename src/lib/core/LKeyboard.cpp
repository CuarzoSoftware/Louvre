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

LKeyboard::LKeyboard(Params *params)
{
    L_UNUSED(params);
    m_imp = new LKeyboardPrivate();
    seat()->imp()->keyboard = this;

    // Create XKB context
    imp()->xkbContext = xkb_context_new(XKB_CONTEXT_NO_FLAGS);

    // Set the default keymap
    setKeymap();
}

LKeyboard::~LKeyboard()
{
    delete m_imp;
}

bool LKeyboard::setKeymap(const char *rules, const char *model, const char *layout, const char *variant, const char *options)
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

    char *keymapString,*map;
    const char *xdgRuntimeDir;

    imp()->xkbKeymapName.rules = rules;
    imp()->xkbKeymapName.model = model;
    imp()->xkbKeymapName.layout = layout;
    imp()->xkbKeymapName.variant = variant;
    imp()->xkbKeymapName.options = options;

    // Find a keymap matching suggestions
    imp()->xkbKeymap = xkb_keymap_new_from_names(imp()->xkbContext, &imp()->xkbKeymapName, XKB_KEYMAP_COMPILE_NO_FLAGS);

    if (!imp()->xkbKeymap)
    {
        LLog::error("[keyboard] Failed to set keymap with names Rules: %s, Model: %s, Layout: %s, Variant: %s, Opetions: %s. Restoring default keymap.",
                    rules, model, layout, variant, options);

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
        printf("Louvre error: XDG_RUNTIME_DIR env not set.\n");
        exit(EXIT_FAILURE);
    }

    // Open and store the file descritor
    imp()->xkbKeymapFd = open(xdgRuntimeDir, O_TMPFILE|O_RDWR|O_EXCL, 0600);

    if (imp()->xkbKeymapFd < 0)
    {
        printf("Error creating shared memory for keyboard layout.\n");
        exit(-1);
    }

    // Write the keymap string
    ftruncate(imp()->xkbKeymapFd, imp()->xkbKeymapSize);
    map = (char*)mmap(NULL, imp()->xkbKeymapSize, PROT_READ|PROT_WRITE, MAP_SHARED, imp()->xkbKeymapFd, 0);
    memcpy(map, keymapString, imp()->xkbKeymapSize);
    munmap(map, imp()->xkbKeymapSize);

    // Keymap string not needed anymore
    free(keymapString);

    // Create a xkb keyboard state to handle modifiers
    imp()->xkbKeymapState = xkb_state_new(imp()->xkbKeymap);

    if (!imp()->xkbKeymapState)
    {
        LLog::error("[keyboard] Failed to get keymap state with names Rules: %s, Model: %s, Layout: %s, Variant: %s, Opetions: %s. Restoring default keymap.",
                    rules, model, layout, variant, options);
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
            LLog::error("[keyboard] Failed to set default keymap. Disabling keymap.");
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

LSurface *LKeyboard::focusSurface() const
{
    return imp()->keyboardFocusSurface;
}

const LKeyboard::KeyboardModifiersState &LKeyboard::modifiersState() const
{
    return imp()->modifiersState;
}

void LKeyboard::setFocus(LSurface *surface)
{
    if (surface)
    {
        // If already has focus
        if (focusSurface() == surface)
            return;
        else
        {
            // If another surface has focus (hack for firefox)
            if (focusSurface())
            {
                UInt32 serial = LCompositor::nextSerial();

                for (Wayland::GSeat *s : focusSurface()->client()->seatGlobals())
                {
                    if (s->keyboardResource())
                    {
                        s->keyboardResource()->imp()->serials.leave = serial;
                        s->keyboardResource()->leave(serial, focusSurface()->surfaceResource());
                        break;
                    }
                }
            }

            if (!focusSurface() || (focusSurface() && focusSurface()->client() != surface->client()))
                surface->client()->dataDevice().sendSelectionEvent();

            // If the new surface has no wl_pointer then it is like calling setFocus(nullptr)
            imp()->keyboardFocusSurface = nullptr;

            UInt32 serial = LCompositor::nextSerial();

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
        if (focusSurface())
        {
            UInt32 serial = LCompositor::nextSerial();
            for (Wayland::GSeat *s : focusSurface()->client()->seatGlobals())
            {
                if (s->keyboardResource())
                {
                    s->keyboardResource()->imp()->serials.leave = serial;
                    s->keyboardResource()->leave(serial, focusSurface()->surfaceResource());
                }
            }
        }
        imp()->keyboardFocusSurface = nullptr;
    }

}

void LKeyboard::sendKeyEvent(UInt32 keyCode, UInt32 keyState)
{
    // If no surface has focus
    if (!focusSurface())
        return;

    UInt32 serial = LCompositor::nextSerial();
    UInt32 ms = LTime::ms();

    for (Wayland::GSeat *s : focusSurface()->client()->seatGlobals())
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
    if (!focusSurface())
        return;

    UInt32 serial = LCompositor::nextSerial();

    for (Wayland::GSeat *s : focusSurface()->client()->seatGlobals())
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

const std::list<UInt32> &LKeyboard::pressedKeys() const
{
    return imp()->pressedKeys;
}

bool LKeyboard::isKeyCodePressed(UInt32 keyCode) const
{
    for (UInt32 key : pressedKeys())
    {
        if (key == keyCode)
            return true;
    }
    return false;
}

bool LKeyboard::LKeyboardPrivate::backendKeyEvent(UInt32 keyCode, UInt32 keyState)
{
    if (xkbKeymapState)
        xkb_state_update_key(xkbKeymapState,
                             keyCode+8,
                             (xkb_key_direction)keyState);

    if (keyState == LKeyboard::Pressed)
        pressedKeys.push_back(keyCode);
    else
        pressedKeys.remove(keyCode);

    LCompositor::compositor()->seat()->keyboard()->keyEvent(keyCode, keyState);
    LCompositor::compositor()->seat()->keyboard()->imp()->updateModifiers();

    // CTRL + ALT + (F1, F2, ..., F10) : Switch TTY.
    if (LCompositor::compositor()->seat()->imp()->libseatHandle &&
        keyCode >= KEY_F1 &&
        keyCode <= KEY_F10 &&
        (
            LCompositor::compositor()->seat()->keyboard()->isModActive(XKB_MOD_NAME_CTRL) ||
            LCompositor::compositor()->seat()->keyboard()->isKeyCodePressed(KEY_LEFTCTRL)
        ) &&
        (
            LCompositor::compositor()->seat()->keyboard()->isModActive(XKB_MOD_NAME_ALT) ||
            LCompositor::compositor()->seat()->keyboard()->isKeyCodePressed(KEY_LEFTALT)
        )
        )
    {
        LCompositor::compositor()->seat()->setTTY(keyCode - KEY_F1 + 1);
        return true;
    }

    return false;
}

void LKeyboard::LKeyboardPrivate::updateModifiers()
{
    if (xkbKeymapState)
    {
        modifiersState.depressed = xkb_state_serialize_mods(xkbKeymapState, XKB_STATE_MODS_DEPRESSED);
        modifiersState.latched = xkb_state_serialize_mods(xkbKeymapState, XKB_STATE_MODS_LATCHED);
        modifiersState.locked = xkb_state_serialize_mods(xkbKeymapState, XKB_STATE_MODS_LOCKED);
        modifiersState.group = xkb_state_serialize_layout(xkbKeymapState, XKB_STATE_LAYOUT_EFFECTIVE);
    }
    seat()->keyboard()->keyModifiersEvent(modifiersState.depressed,
                                          modifiersState.latched,
                                          modifiersState.locked,
                                          modifiersState.group);
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
