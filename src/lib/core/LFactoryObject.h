#ifndef LFACTORYOBJECT_H
#define LFACTORYOBJECT_H

#include <LObject.h>

/**
 * @brief Base class for Factory objects.
 *
 * @section Factory
 *
 * A Wayland compositor usually handles many different tasks such as processing
 * client requests from various protocols, managing input events, rendering, and
 * more.
 *
 * In Louvre, each functionality is managed by a specific class, typically
 * through virtual methods.\n This approach allows Louvre to provide a default
 * way to handle each functionality while also enabling developers to override
 * it when desired, which is known as the factory pattern.
 *
 * These classes are derived from this base class and their lifetime is managed
 * entirely by LCompositor.\n When LCompositor::createObjectRequest() is
 * triggered, it expects you to return a new instance of a specific
 * LFactoryObject subtype. If `nullptr` is returned, Louvre creates and uses an
 * instance of the default class.
 *
 * @see Type to see all classes that can be overridden.
 *
 * Before an instance of a class is destroyed by the compositor, the
 * LCompositor::onAnticipatedObjectDestruction() event is triggered. This is
 * called well in advance of the actual object's destructor, allowing you to
 * still access many of its related resources.
 *
 * @note The compositor is responsible for destroying the object, do not attempt
 * not delete it yourself.
 */
class Louvre::LFactoryObject : public LObject {
 public:
  /**
   * @brief Base factory object types.
   *
   * Enum representing all Louvre classes that can be overridden.
   * The names match the names of the actual classes.
   *
   * @see LCompositor::createObjectRequest()
   */
  enum class Type : Int32 {
    /// Represents the LSurface class.
    LSurface,

    /// Represents the LToplevelRole class.
    LToplevelRole,

    /// Represents the LForeignToplevelController class.
    LForeignToplevelController,

    /// Represents the LPopupRole class.
    LPopupRole,

    /// Represents the LSubsurfaceRole class.
    LSubsurfaceRole,

    /// Represents the LCursorRole class.
    LCursorRole,

    /// Represents the LDNDIconRole class.
    LDNDIconRole,

    /// Represents the LSessionLockRole class.
    LSessionLockRole,

    /// Represents the LLayerRole class.
    LLayerRole,

    /// Represents the LClient class.
    LClient,

    /// Represents the LOutput class.
    LOutput,

    /// Represents the LSeat class.
    LSeat,

    /// Represents the LPointer class.
    LPointer,

    /// Represents the LKeyboard class.
    LKeyboard,

    /// Represents the LTouch class.
    LTouch,

    /// Represents the LClipboard class.
    LClipboard,

    /// Represents the LDND class.
    LDND,

    /// Represents the LSessionLockManager class.
    LSessionLockManager,

    /// Represents the LActivationTokenManager class.
    LActivationTokenManager,

    /// Represents the LBackgroundBlur class.
    LBackgroundBlur
  };

  /**
   * @brief Gets the base factory object type.
   *
   * Returns the base factory object type associated with this instance.
   *
   * @return The base factory object type.
   */
  Type factoryObjectType() const noexcept { return m_type; }

 protected:
  LFactoryObject(Type type) noexcept;

 private:
  Type m_type;
};

#endif  // LFACTORYOBJECT_H
