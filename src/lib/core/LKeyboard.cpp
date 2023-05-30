#include <protocols/Wayland/GSeat.h>
#include <protocols/Wayland/RKeyboard.h>
#include <private/LClientPrivate.h>
#include <private/LDataDevicePrivate.h>
#include <private/LKeyboardPrivate.h>

#include <LTime.h>
#include <LCompositor.h>
#include <LSeat.h>
#include <LDNDManager.h>
#include <LClient.h>
#include <LCursor.h>
#include <LOutput.h>

#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

using namespace Louvre;

LKeyboard::LKeyboard(Params *params)
{
    m_imp = new LKeyboardPrivate();
    imp()->keyboard = this;
    imp()->seat = params->seat;

    // Create null keys
    wl_array_init(&imp()->keys);

    // Create XKB context
    imp()->xkbContext = xkb_context_new(XKB_CONTEXT_NO_FLAGS);

    // Set the default keymap
    setKeymap();
}

LKeyboard::~LKeyboard()
{
    wl_array_release(&imp()->keys);
    delete m_imp;
}

LSeat *LKeyboard::seat() const
{
    return imp()->seat;
}

LCompositor *LKeyboard::compositor() const
{
    return seat()->compositor();
}

void LKeyboard::setKeymap(const char *rules, const char *model, const char *layout, const char *variant, const char *options)
{
    if (imp()->xkbKeymapFd != -1)
    {
         close(imp()->xkbKeymapFd);
    }

    char *keymapString,*map;

    //const char *xdgRuntimeDir = "/run/user/1000";

    imp()->xkbKeymapName.rules = rules;
    imp()->xkbKeymapName.model = model;
    imp()->xkbKeymapName.layout = layout;
    imp()->xkbKeymapName.variant = variant;
    imp()->xkbKeymapName.options = options;

    // Find a keymap matching suggestions
    imp()->xkbKeymap = xkb_keymap_new_from_names(imp()->xkbContext, &imp()->xkbKeymapName, XKB_KEYMAP_COMPILE_NO_FLAGS);

    // Get the keymap string
    keymapString = xkb_keymap_get_as_string(imp()->xkbKeymap, XKB_KEYMAP_FORMAT_TEXT_V1);

    // Store the keymap size
    imp()->xkbKeymapSize = strlen(keymapString) + 1;

    // Get the XDG_RUNTIME_DIR env
    const char *xdgRuntimeDir = getenv("XDG_RUNTIME_DIR");

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
    int dummy = ftruncate(imp()->xkbKeymapFd, imp()->xkbKeymapSize);
    L_UNUSED(dummy);
    map = (char*)mmap(NULL, imp()->xkbKeymapSize, PROT_READ|PROT_WRITE, MAP_SHARED, imp()->xkbKeymapFd, 0);
    memcpy(map,keymapString,imp()->xkbKeymapSize);
    munmap(map, imp()->xkbKeymapSize);

    // Keymap string not needed anymore
    free(keymapString);

    // Create a xkb keyboard state to handle modifiers
    imp()->xkbKeymapState = xkb_state_new(imp()->xkbKeymap);

    for (LClient *c : compositor()->clients())
    {
        for (Protocols::Wayland::GSeat *s : c->seatGlobals())
        {
            if (s->keyboardResource())
                s->keyboardResource()->sendKeymap(keymapFd(), keymapSize());
        }
    }

}

Int32 LKeyboard::keymapFd() const
{
    return imp()->xkbKeymapFd;
}

Int32 LKeyboard::keymapSize() const
{
    return imp()->xkbKeymapSize;
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
    // If surface is not nullptr
    if (surface)
    {

        // If already has focus
        if (focusSurface() == surface)
            return;
        else
        {
            // If another surface has focus
            if (focusSurface())
            {
                for (Protocols::Wayland::GSeat *s : focusSurface()->client()->seatGlobals())
                    if (s->keyboardResource())
                        s->keyboardResource()->sendLeave(focusSurface());
            }


            if (!focusSurface() || (focusSurface() && focusSurface()->client() != surface->client()))
                surface->client()->dataDevice().sendSelectionEvent();


            bool hasRKeyboard = false;

            for (Protocols::Wayland::GSeat *s : surface->client()->seatGlobals())
            {
                if (s->keyboardResource())
                {
                    hasRKeyboard = true;
                    s->keyboardResource()->sendEnter(surface);
                    s->keyboardResource()->sendModifiers(modifiersState().depressed, modifiersState().latched, modifiersState().locked, modifiersState().group);
                }
            }

            if (hasRKeyboard)
                imp()->keyboardFocusSurface = surface;
            else
                imp()->keyboardFocusSurface = nullptr;

        }
    }
    else
    {
        // Remove focus from current surface
        if (focusSurface())
        {
            for (Protocols::Wayland::GSeat *s : focusSurface()->client()->seatGlobals())
                if (s->keyboardResource())
                    s->keyboardResource()->sendLeave(focusSurface());
        }
        imp()->keyboardFocusSurface = nullptr;
    }
}

void LKeyboard::sendKeyEvent(UInt32 keyCode, UInt32 keyState)
{
    // If no surface has focus
    if (!focusSurface())
        return;

    for (Protocols::Wayland::GSeat *s : focusSurface()->client()->seatGlobals())
        if (s->keyboardResource())
            s->keyboardResource()->sendKey(keyCode, keyState);
}

void LKeyboard::sendModifiersEvent(UInt32 depressed, UInt32 latched, UInt32 locked, UInt32 group)
{
    // If no surface has focus
    if (!focusSurface())
        return;

    for (Protocols::Wayland::GSeat *s : focusSurface()->client()->seatGlobals())
        if (s->keyboardResource())
            s->keyboardResource()->sendModifiers(depressed, latched, locked, group);
}

void LKeyboard::sendModifiersEvent()
{
    sendModifiersEvent(imp()->modifiersState.depressed, imp()->modifiersState.latched, imp()->modifiersState.locked, imp()->modifiersState.group);
}

xkb_keysym_t LKeyboard::keySymbol(UInt32 keyCode)
{
    return xkb_state_key_get_one_sym(imp()->xkbKeymapState,keyCode+8);
}

xkb_state *LKeyboard::keymapState() const
{
    return imp()->xkbKeymapState;
}

bool LKeyboard::isModActive(const char *name) const
{
    return xkb_state_mod_name_is_active(
                imp()->xkbKeymapState,
                name,
                XKB_STATE_MODS_EFFECTIVE);
}

void LKeyboard::LKeyboardPrivate::updateModifiers()
{
    modifiersState.depressed = xkb_state_serialize_mods(xkbKeymapState, XKB_STATE_MODS_DEPRESSED);
    modifiersState.latched = xkb_state_serialize_mods(xkbKeymapState, XKB_STATE_MODS_LATCHED);
    modifiersState.locked = xkb_state_serialize_mods(xkbKeymapState, XKB_STATE_MODS_LOCKED);
    modifiersState.group = xkb_state_serialize_layout(xkbKeymapState, XKB_STATE_LAYOUT_EFFECTIVE);
    keyboard->keyModifiersEvent(modifiersState.depressed,modifiersState.latched,modifiersState.locked,modifiersState.group);
}

#if LOUVRE_SEAT_VERSION >= 4

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
                    s->keyboardResource()->sendRepeatInfo(rate, msDelay);
        }
    }

#endif
