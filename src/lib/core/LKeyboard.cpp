#include <LClient.h>
#include <LClipboard.h>
#include <LCompositor.h>
#include <LCursor.h>
#include <LDND.h>
#include <LLog.h>
#include <LObject.h>
#include <LOutput.h>
#include <LTime.h>
#include <fcntl.h>
#include <private/LClientPrivate.h>
#include <private/LKeyboardPrivate.h>
#include <private/LSeatPrivate.h>
#include <private/LSurfacePrivate.h>
#include <protocols/Wayland/GSeat.h>
#include <protocols/Wayland/RDataDevice.h>
#include <protocols/Wayland/RDataOffer.h>
#include <protocols/Wayland/RKeyboard.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include <cassert>

using namespace Louvre::Protocols::Wayland;

LKeyboard::LKeyboard(const void *params) noexcept
    : LFactoryObject(FactoryObjectType), LPRIVATE_INIT_UNIQUE(LKeyboard) {
  assert(params != nullptr &&
         "Invalid parameter passed to LKeyboard constructor.");
  LKeyboard **ptr{(LKeyboard **)params};
  assert(*ptr == nullptr && *ptr == seat()->keyboard() &&
         "Only a single LKeyboard instance can exist.");
  *ptr = this;

  // Create XKB context
  imp()->xkbContext = xkb_context_new(XKB_CONTEXT_NO_FLAGS);

  // Set the default keymap
  setKeymap();

  imp()->focus.setOnDestroyCallback([this](auto) { focusChanged(); });
}

LKeyboard::~LKeyboard() {
  notifyDestruction();

  if (imp()->xkbKeymapFd != -1) {
    close(imp()->xkbKeymapFd);
    imp()->xkbKeymapFd = -1;
  }

  if (imp()->xkbKeymapState) {
    xkb_state_unref(imp()->xkbKeymapState);
    imp()->xkbKeymapState = nullptr;
  }

  if (imp()->xkbKeymap) {
    xkb_keymap_unref(imp()->xkbKeymap);
    imp()->xkbKeymap = nullptr;
  }

  if (imp()->xkbContext) {
    xkb_context_unref(imp()->xkbContext);
    imp()->xkbContext = nullptr;
  }
}

void LKeyboard::setGrab(LSurface *surface) {
  if (imp()->grab == surface) return;

  if (imp()->focus == surface) imp()->focus.reset();

  imp()->grab.reset();
  setFocus(surface);
  imp()->grab.reset(surface);
}

LSurface *LKeyboard::grab() const noexcept { return imp()->grab; }

const LKeyboardModifiersEvent::Modifiers &LKeyboard::modifiers()
    const noexcept {
  return imp()->currentModifiersState;
}

bool LKeyboard::setKeymap(const char *rules, const char *model,
                          const char *layout, const char *variant,
                          const char *options) noexcept {
  static constexpr const char *METHOD_NAME = "LKeyboard::setKeymap";

  if (imp()->xkbKeymapFd != -1) {
    close(imp()->xkbKeymapFd);
    imp()->xkbKeymapFd = -1;
  }

  if (imp()->xkbKeymapState) {
    xkb_state_unref(imp()->xkbKeymapState);
    imp()->xkbKeymapState = nullptr;
  }

  if (imp()->xkbKeymap) {
    xkb_keymap_unref(imp()->xkbKeymap);
    imp()->xkbKeymap = nullptr;
  }

  char *keymapString, *map;
  const char *xdgRuntimeDir;
  int res;

  imp()->xkbKeymapName.rules = rules;
  imp()->xkbKeymapName.model = model;
  imp()->xkbKeymapName.layout = layout;
  imp()->xkbKeymapName.variant = variant;
  imp()->xkbKeymapName.options = options;

  // Find a keymap matching suggestions
  imp()->xkbKeymap = xkb_keymap_new_from_names(
      imp()->xkbContext, &imp()->xkbKeymapName, XKB_KEYMAP_COMPILE_NO_FLAGS);

  if (!imp()->xkbKeymap) goto fail;

  // Get the keymap string
  keymapString =
      xkb_keymap_get_as_string(imp()->xkbKeymap, XKB_KEYMAP_FORMAT_TEXT_V1);

  // Store the keymap size
  imp()->xkbKeymapSize = strlen(keymapString) + 1;

  // Get the XDG_RUNTIME_DIR env
  xdgRuntimeDir = getenv("XDG_RUNTIME_DIR");

  if (!xdgRuntimeDir) {
    LLog::error("[%s] XDG_RUNTIME_DIR env not set. Using /tmp,", METHOD_NAME);
    xdgRuntimeDir = "/tmp";
  }

  // Open and store the file descritor
  imp()->xkbKeymapFd = open(xdgRuntimeDir, O_TMPFILE | O_RDWR | O_EXCL, 0600);

  if (imp()->xkbKeymapFd < 0) {
    LLog::error("[%s] Failed to allocate shared memory for keymap.",
                METHOD_NAME);
    goto fail;
  }

  // Write the keymap string
  res = ftruncate(imp()->xkbKeymapFd, imp()->xkbKeymapSize);
  L_UNUSED(res);
  map = (char *)mmap(NULL, imp()->xkbKeymapSize, PROT_READ | PROT_WRITE,
                     MAP_SHARED, imp()->xkbKeymapFd, 0);
  memcpy(map, keymapString, imp()->xkbKeymapSize);
  munmap(map, imp()->xkbKeymapSize);

  // Keymap string not needed anymore
  free(keymapString);

  // Create a xkb keyboard state to handle modifiers
  imp()->xkbKeymapState = xkb_state_new(imp()->xkbKeymap);

  if (!imp()->xkbKeymapState) goto fail;

  imp()->keymapFormat = WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1;

  for (auto client : compositor()->clients())
    for (auto gSeat : client->seatGlobals())
      for (auto rKeyboard : gSeat->keyboardRes())
        rKeyboard->keymap(keymapFormat(), keymapFd(), keymapSize());

  // Update LED idx
  imp()->leds[0] = xkb_keymap_led_get_index(imp()->xkbKeymap, XKB_LED_NAME_NUM);
  imp()->leds[1] =
      xkb_keymap_led_get_index(imp()->xkbKeymap, XKB_LED_NAME_CAPS);
  imp()->leds[2] =
      xkb_keymap_led_get_index(imp()->xkbKeymap, XKB_LED_NAME_SCROLL);

  return true;

fail:

  LLog::error(
      "[%s] Failed to set keymap with names Rules: %s, Model: %s, Layout: %s, "
      "Variant: %s, Opetions: %s. Trying XKB_DEFAULT envs or fallback keymap.",
      METHOD_NAME, rules ? rules : getenv("XKB_DEFAULT_RULES"),
      model ? model : getenv("XKB_DEFAULT_MODEL"),
      layout ? layout : getenv("XKB_DEFAULT_LAYOUT"),
      variant ? variant : getenv("XKB_DEFAULT_VARIANT"),
      options ? options : getenv("XKB_DEFAULT_OPTIONS"));

  if (rules != NULL || model != NULL || layout != NULL || variant != NULL ||
      options != NULL) {
    if (setKeymap())
      return false;
    else
      LLog::error(
          "[%s] Failed to set keymap from XKB_DEFAULT envs. Using fallback "
          "keymap.",
          METHOD_NAME);
  }

  if (getenv("XKB_DEFAULT_RULES") || getenv("XKB_DEFAULT_MODEL") ||
      getenv("XKB_DEFAULT_LAYOUT") || getenv("XKB_DEFAULT_VARIANT") ||
      getenv("XKB_DEFAULT_OPTIONS")) {
    unsetenv("XKB_DEFAULT_RULES");
    unsetenv("XKB_DEFAULT_MODEL");
    unsetenv("XKB_DEFAULT_LAYOUT");
    unsetenv("XKB_DEFAULT_VARIANT");
    unsetenv("XKB_DEFAULT_OPTIONS");

    if (setKeymap())
      return false;
    else
      LLog::error("[%s] No keymap could be found. Disabling keymap.",
                  METHOD_NAME);
  }

  // Worst case, disables keymap

  imp()->xkbKeymapSize = 0;

  if (imp()->xkbKeymapFd != -1) {
    close(imp()->xkbKeymapFd);
    imp()->xkbKeymapFd = -1;
  }

  if (imp()->xkbKeymapState) {
    xkb_state_unref(imp()->xkbKeymapState);
    imp()->xkbKeymapState = nullptr;
  }

  if (imp()->xkbKeymap) {
    xkb_keymap_unref(imp()->xkbKeymap);
    imp()->xkbKeymap = nullptr;
  }

  imp()->xkbKeymapFd = open("/dev/null", O_RDONLY);

  imp()->keymapFormat = WL_KEYBOARD_KEYMAP_FORMAT_NO_KEYMAP;

  for (auto client : compositor()->clients())
    for (auto gSeat : client->seatGlobals())
      for (auto rKeyboard : gSeat->keyboardRes())
        rKeyboard->keymap(keymapFormat(), keymapFd(), keymapSize());

  return false;
}

Int32 LKeyboard::keymapFd() const noexcept { return imp()->xkbKeymapFd; }

Int32 LKeyboard::keymapSize() const noexcept { return imp()->xkbKeymapSize; }

UInt32 LKeyboard::keymapFormat() const noexcept { return imp()->keymapFormat; }

LSurface *LKeyboard::focus() const noexcept { return imp()->focus; }

void LKeyboard::setFocus(LSurface *surface) {
  if (imp()->grab) return;

  LWeak<LSurface> prev{focus()};

  if (surface) {
    if (focus() == surface) return;

    if (focus()) {
      LKeyboardLeaveEvent leaveEvent;

      for (auto gSeat : focus()->client()->seatGlobals())
        for (auto rKeyboard : gSeat->keyboardRes())
          rKeyboard->leave(leaveEvent, focus()->surfaceResource());
    }

    const bool clientChanged{!seat()->clipboard()->m_dataOffer ||
                             seat()->clipboard()->m_dataOffer->client() !=
                                 surface->client()};

    if (clientChanged && seat()->clipboard()->m_dataOffer &&
        seat()->clipboard()->m_dataOffer->dataDeviceRes())
      seat()->clipboard()->m_dataOffer->dataDeviceRes()->selection(nullptr);

    imp()->focus.reset();

    // Pack currently pressed keys
    wl_array keys;
    wl_array_init(&keys);

    for (UInt32 key : seat()->keyboard()->pressedKeys())
      *(UInt32 *)wl_array_add(&keys, sizeof(UInt32)) = key;

    LKeyboardEnterEvent enterEvent;
    LKeyboardModifiersEvent modifiersEvent{modifiers()};
    bool selectionSent{false};

    for (auto gSeat : surface->client()->seatGlobals()) {
      if (clientChanged && !selectionSent && gSeat->dataDeviceRes()) {
        gSeat->dataDeviceRes()->createOffer(RDataSource::Clipboard);
        selectionSent = true;
      }

      for (auto rKeyboard : gSeat->keyboardRes()) {
        imp()->focus.reset(surface);
        rKeyboard->enter(enterEvent, surface->surfaceResource(), &keys);
        rKeyboard->modifiers(modifiersEvent);
      }
    }

    wl_array_release(&keys);
  } else {
    // Remove focus from current surface
    if (focus()) {
      LKeyboardLeaveEvent leaveEvent;

      for (auto gSeat : focus()->client()->seatGlobals())
        for (auto rKeyboard : gSeat->keyboardRes())
          rKeyboard->leave(leaveEvent, focus()->surfaceResource());
    }

    if (seat()->clipboard()->m_dataOffer &&
        seat()->clipboard()->m_dataOffer->dataDeviceRes())
      seat()->clipboard()->m_dataOffer->dataDeviceRes()->selection(nullptr);

    imp()->focus.reset();
  }

  if (prev.get() != focus()) focusChanged();
}

void LKeyboard::sendKeyEvent(const LKeyboardKeyEvent &event) noexcept {
  if (!focus()) return;

  const LKeyboardModifiersEvent modifiersEvent{modifiers()};

  for (auto gSeat : focus()->client()->seatGlobals()) {
    for (auto rKeyboard : gSeat->keyboardRes()) {
      rKeyboard->key(event);

      if (imp()->modifiersChanged) rKeyboard->modifiers(modifiersEvent);
    }
  }
}

xkb_keysym_t LKeyboard::keySymbol(UInt32 keyCode) noexcept {
  if (!imp()->xkbKeymapState) return keyCode;

  return xkb_state_key_get_one_sym(imp()->xkbKeymapState, keyCode + 8);
}

xkb_state *LKeyboard::keymapState() const noexcept {
  return imp()->xkbKeymapState;
}

bool LKeyboard::isModActive(const char *name,
                            xkb_state_component type) const noexcept {
  if (!imp()->xkbKeymapState) return false;

  return xkb_state_mod_name_is_active(imp()->xkbKeymapState, name, type) == 1;
}

const std::vector<UInt32> &LKeyboard::pressedKeys() const noexcept {
  return imp()->pressedKeys;
}

bool LKeyboard::isKeyCodePressed(UInt32 keyCode) const noexcept {
  for (UInt32 key : imp()->pressedKeys)
    if (key == keyCode) return true;

  return false;
}

bool LKeyboard::isKeySymbolPressed(xkb_keysym_t keySymbol) const noexcept {
  for (UInt32 key : imp()->pressedKeys)
    if (xkb_state_key_get_one_sym(imp()->xkbKeymapState, key + 8) == keySymbol)
      return true;

  return false;
}

// Since 4

Int32 LKeyboard::repeatRate() const noexcept { return imp()->repeatRate; }

Int32 LKeyboard::repeatDelay() const noexcept { return imp()->repeatDelay; }

void LKeyboard::setRepeatInfo(Int32 rate, Int32 msDelay) noexcept {
  imp()->repeatRate = rate < 0 ? 0 : rate;
  imp()->repeatDelay = msDelay < 0 ? 0 : msDelay;

  for (auto client : compositor()->clients())
    for (auto gSeat : client->seatGlobals())
      for (auto rKeyboard : gSeat->keyboardRes())
        rKeyboard->repeatInfo(rate, msDelay);
}
