#include <private/LClientPrivate.h>
#include <private/LDataDevicePrivate.h>
#include <private/LKeyboardPrivate.h>

#include <LTime.h>
#include <LWayland.h>
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
    m_imp->keyboard = this;
    m_imp->seat = params->seat;

    // Create null keys
    wl_array_init(&m_imp->keys);

    // Create XKB context
    m_imp->xkbContext = xkb_context_new(XKB_CONTEXT_NO_FLAGS);

    // Set the default keymap
    setKeymap();
}

LKeyboard::~LKeyboard()
{
    wl_array_release(&m_imp->keys);
    delete m_imp;
}

LSeat *LKeyboard::seat() const
{
    return m_imp->seat;
}

LCompositor *LKeyboard::compositor() const
{
    return seat()->compositor();
}

void LKeyboard::setKeymap(const char *rules, const char *model, const char *layout, const char *variant, const char *options)
{
    if(m_imp->xkbKeymapFd != -1)
    {
         close(m_imp->xkbKeymapFd);
    }

    char *keymapString,*map;

    //const char *xdgRuntimeDir = "/run/user/1000";

    m_imp->xkbKeymapName.rules = rules;
    m_imp->xkbKeymapName.model = model;
    m_imp->xkbKeymapName.layout = layout;
    m_imp->xkbKeymapName.variant = variant;
    m_imp->xkbKeymapName.options = options;

    // Find a keymap matching suggestions
    m_imp->xkbKeymap = xkb_keymap_new_from_names(m_imp->xkbContext, &m_imp->xkbKeymapName, XKB_KEYMAP_COMPILE_NO_FLAGS);

    // Get the keymap string
    keymapString = xkb_keymap_get_as_string(m_imp->xkbKeymap, XKB_KEYMAP_FORMAT_TEXT_V1);

    // Store the keymap size
    m_imp->xkbKeymapSize = strlen(keymapString) + 1;

    // Get the XDG_RUNTIME_DIR env
    const char *xdgRuntimeDir = getenv("XDG_RUNTIME_DIR");

    if(!xdgRuntimeDir)
    {
        printf("Louvre error: XDG_RUNTIME_DIR env not set.\n");
        exit(EXIT_FAILURE);
    }

    // Open and store the file descritor
    m_imp->xkbKeymapFd = open(xdgRuntimeDir, O_TMPFILE|O_RDWR|O_EXCL, 0600);

    if(m_imp->xkbKeymapFd < 0)
    {
        printf("Error creating shared memory for keyboard layout.\n");
        exit(-1);
    }

    // Write the keymap string
    int dummy = ftruncate(m_imp->xkbKeymapFd, m_imp->xkbKeymapSize);
    L_UNUSED(dummy);
    map = (char*)mmap(NULL, m_imp->xkbKeymapSize, PROT_READ|PROT_WRITE, MAP_SHARED, m_imp->xkbKeymapFd, 0);
    memcpy(map,keymapString,m_imp->xkbKeymapSize);
    munmap(map, m_imp->xkbKeymapSize);

    // Keymap string not needed anymore
    free(keymapString);

    // Create a xkb keyboard state to handle modifiers
    m_imp->xkbKeymapState = xkb_state_new(m_imp->xkbKeymap);

    for(LClient *lClient : compositor()->clients())
    {
        if(lClient->keyboardResource())
            wl_keyboard_send_keymap(lClient->keyboardResource(),WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1, keymapFd(), keymapSize());
    }
}

Int32 LKeyboard::keymapFd() const
{
    return m_imp->xkbKeymapFd;
}

Int32 LKeyboard::keymapSize() const
{
    return m_imp->xkbKeymapSize;
}

LSurface *LKeyboard::focusSurface() const
{
    return m_imp->keyboardFocusSurface;
}

const LKeyboard::KeyboardModifiersState &LKeyboard::modifiersState() const
{
    return m_imp->modifiersState;
}

void LKeyboard::setFocus(LSurface *surface)
{
    // If surface is not nullptr
    if(surface)
    {

        // If already has focus
        if(focusSurface() == surface)
            return;
        else
        {
            // If another surface has focus
            if(focusSurface() && focusSurface()->client()->keyboardResource())
            {
                focusSurface()->client()->imp()->serials.lastKeyboardLeaveEventSerial = LWayland::nextSerial();
                wl_keyboard_send_leave(focusSurface()->client()->keyboardResource(),focusSurface()->client()->imp()->serials.lastKeyboardLeaveEventSerial,focusSurface()->resource());
            }

            if(surface->client()->keyboardResource())
            {
                // Send clipboard selection event
                if(surface->client()->dataDevice())
                {
                    if(!focusSurface() || (focusSurface() && focusSurface()->client() != surface->client()))
                        surface->client()->dataDevice()->sendSelectionEvent();
                }

                surface->client()->imp()->serials.lastKeyboardEnterEventSerial = LWayland::nextSerial();
                wl_keyboard_send_enter(surface->client()->keyboardResource(),surface->client()->imp()->serials.lastKeyboardKeyEventSerial,surface->resource(),&m_imp->keys);
                m_imp->keyboardFocusSurface = surface;
                sendModifiersEvent();

            }
            else
                m_imp->keyboardFocusSurface = nullptr;
        }

    }
    else
    {
        // If a surface has focus
        if(focusSurface() && focusSurface()->client()->keyboardResource())
        {
            focusSurface()->client()->imp()->serials.lastKeyboardLeaveEventSerial = LWayland::nextSerial();
            wl_keyboard_send_leave(focusSurface()->client()->keyboardResource(),focusSurface()->client()->imp()->serials.lastKeyboardLeaveEventSerial,focusSurface()->resource());
        }
        m_imp->keyboardFocusSurface = nullptr;
    }
}

void LKeyboard::sendKeyEvent(UInt32 keyCode, UInt32 keyState)
{
    // If no surface has focus
    if(!focusSurface())
        return;

    // If do not have a wl_keyboard
    if(!focusSurface()->client()->keyboardResource())
        return;

    focusSurface()->client()->imp()->serials.lastKeyboardKeyEventSerial = LWayland::nextSerial();
    wl_keyboard_send_key(focusSurface()->client()->keyboardResource(),focusSurface()->client()->imp()->serials.lastKeyboardKeyEventSerial,LTime::ms(),keyCode,keyState);
}

void LKeyboard::sendModifiersEvent(UInt32 depressed, UInt32 latched, UInt32 locked, UInt32 group)
{
    // If no surface has focus
    if(!focusSurface())
        return;

    // If do not have a wl_keyboard
    if(!focusSurface()->client()->keyboardResource())
        return;

    focusSurface()->client()->imp()->serials.lastKeyboardModifiersEventSerial = LWayland::nextSerial();
    wl_keyboard_send_modifiers(focusSurface()->client()->keyboardResource(),focusSurface()->client()->imp()->serials.lastKeyboardModifiersEventSerial,depressed,latched,locked,group);
}

void LKeyboard::sendModifiersEvent()
{
    sendModifiersEvent(m_imp->modifiersState.depressed, m_imp->modifiersState.latched, m_imp->modifiersState.locked, m_imp->modifiersState.group);
}

xkb_keysym_t LKeyboard::keySymbol(UInt32 keyCode)
{
    return xkb_state_key_get_one_sym(m_imp->xkbKeymapState,keyCode+8);
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
        return m_imp->repeatRate;
    }

    Int32 LKeyboard::repeatDelay() const
    {
        return m_imp->repeatDelay;
    }

    void LKeyboard::setRepeatInfo(Int32 rate, Int32 msDelay)
    {
        if(rate < 0)
            imp()->repeatRate = 0;
        else
            imp()->repeatRate = rate;

        if(msDelay < 0)
            imp()->repeatDelay = 0;
        else
            imp()->repeatDelay = msDelay;

        for(LClient *lClient : compositor()->clients())
            if(lClient->keyboardResource() && wl_resource_get_version(lClient->keyboardResource()) >= 4)
                wl_keyboard_send_repeat_info(lClient->keyboardResource(),rate,msDelay);

    }

#endif


LKeyboard::LKeyboardPrivate *LKeyboard::imp() const
{
    return m_imp;
}
