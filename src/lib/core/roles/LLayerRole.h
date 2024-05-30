#ifndef LLAYERROLE_H
#define LLAYERROLE_H

#include <LBaseSurfaceRole.h>
#include <LExclusiveZone.h>
#include <LBitset.h>
#include <LOutput.h>
#include <LMargins.h>
#include <LLayout.h>

/**
 * @brief Layer role for surfaces.
 *
 * The LLayerRole is used by clients to create various desktop shell components such as a wallpaper, panels, docks, notifications, etc.\n
 * It is part of the [wlr_layer_shell protocol](https://wayland.app/protocols/wlr-layer-shell-unstable-v1).
 * For a comprehensive understanding of each concept, refer to the protocol documentation.
 *
 * @htmlonly
 * <iframe style="width:100%;height:55vw;max-height:512px" src="https://www.youtube.com/embed/lS9wvJtjbYw?si=gDiJWK9zKBi_VTGo" title="YouTube video player" frameborder="0" allow="accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture; web-share" referrerpolicy="strict-origin-when-cross-origin" allowfullscreen></iframe>
 * @endhtmlonly
 *
 * @section layer-role-layer Layer
 *
 * Clients can assign LLayerRole surfaces to different layers, specifically the Background, Bottom, Top, or Overlay layers as defined in @ref LSurfaceLayer.
 *
 * @see LSurface::layer() and LSurface::layerChanged().
 *
 * @section layer-role-pos Positioning
 *
 * LLayerRole surfaces are always positioned relative to a specific LOutput, which can be defined by the client at creation time or replaced later with
 * setExclusiveOutput(). If the client does not specify one, exclusiveOutput() is initially `nullptr`. The default implementation of configureRequest() assigns
 * the current cursor output in such cases.
 *
 * Clients can anchor the surface to one or multiple edges of the output with optional additional margins. To simplify the position calculation,
 * an instance of LExclusiveZone is used, whose LExclusiveZone::rect() property doesn't always represent an exclusive zone.\n
 * For example, if exclusiveZoneSize() is 0, it provides the space where the surface should be placed to prevent occluding other exclusive zones.
 *
 * @see The default implementation of rolePos() and LExclusiveZone::rect().
 *
 * @section layer-role-exclusive-zone Exclusive Zone
 *
 * LLayerRole surfaces can define a size from the edge they are anchored to, to be considered as an exclusive zone, requesting the compositor
 * to prevent occlusion by other surfaces. Since each LLayerRole has its own LExclusiveZone instance, the LOutput::availableGeometry()
 * is automatically updated, which can be used, for example, to properly configure LToplevelRole or other types of surfaces.
 *
 * @section layer-role-size Size
 *
 * Upon creation, clients invoke configureRequest(), where they expect the compositor to configure the surface with an appropriate size.\n
 * During the request, the size() property contains the size desired by the client. If one of its components is 0, it means
 * the compositor should determine it.
 *
 * @see configureRequest().
 */
class Louvre::LLayerRole : public LBaseSurfaceRole
{
public:

    struct Params;

    static constexpr LFactoryObject::Type FactoryObjectType = LFactoryObject::Type::LLayerRole;

    /**
     * @brief Defines the keyboard interactivity modes for a surface.
     *
     * This enumeration specifies the different modes of keyboard interaction that a surface can have.
     *
     * @see keyboardInteractivity()
     */
    enum KeyboardInteractivity
    {
        /// The surface should not receive keyboard focus.
        NoInteractivity = 0,

        /// The surface should grab and hold the keyboard focus exclusively.
        /// @see LKeyboard::setGrab()
        Exclusive = 1,

        /// The surface should follow normal keyboard focus rules.
        OnDemand = 2
    };

    /**
     * @brief Indicates which properties have changed during an atomsChanged() event.
     */
    enum AtomChanges : UInt32
    {
        /// Indicates that size() has changed.
        SizeChanged                     = static_cast<UInt32>(1) << 0,

        /// Indicates that anchor() has changed.
        AnchorChanged                   = static_cast<UInt32>(1) << 1,

        /// Indicates that the exclusiveZoneSize() has changed.
        ExclusiveZoneSizeChanged            = static_cast<UInt32>(1) << 2,

        /// Indicates that margins() have changed.
        MarginsChanged                   = static_cast<UInt32>(1) << 3,

        /// Indicates that keyboardInteractivity() has changed.
        KeyboardInteractivityChanged    = static_cast<UInt32>(1) << 4,

        /// Indicates that the layer() has changed.
        LayerChanged                    = static_cast<UInt32>(1) << 5,

        /// Indicates that the exclusiveEdge() has changed.
        ExclusiveEdgeChanged            = static_cast<UInt32>(1) << 6,
    };

    /**
     * @brief Atomically changing properties.
     *
     * This struct contains all properties related to LLayerRole whose changes should be handled atomically.
     *
     * Using individual property change event listeners could cause issues related to the order in which they are emitted.\n
     * Therefore, in Wayland, the concept of a double-buffered state is often used, where current properties are replaced by
     * pending properties atomically.
     *
     * @see atoms()
     * @see atomsChanged()
     */
    struct Atoms
    {
        /// size()
        LSize size;

        /// anchor()
        LBitset<LEdge> anchor;

        /// exclusiveZoneSize()
        Int32 exclusiveZoneSize;

        /// margins()
        LMargins margins;

        /// keyboardInteractivity()
        KeyboardInteractivity keyboardInteractivity { NoInteractivity };

        /// exclusiveEdge()
        LEdge exclusiveEdge { LEdgeNone };

        /// layer()
        LSurfaceLayer layer;
    };

    /**
     * @brief Constructor of the LLayerRole class.
     *
     * @param params Internal parameters provided in LCompositor::createObjectRequest().
     */
    LLayerRole(const void *params) noexcept;

    LCLASS_NO_COPY(LLayerRole)

    /**
     * @brief Destructor of the LLayerRole class.
     *
     * Invoked after LCompositor::onAnticipatedObjectDestruction().
     */
    ~LLayerRole() noexcept;

    /**
     * @brief LLayerRole surface position.
     *
     * The default implementation positions the surface relative to its current exclusiveOutput(),
     * taking into account its current anchors(), margins(), and the information provided by its exclusiveZone().
     *
     * #### Default Implementation
     * @snippet LLayerRoleDefault.cpp rolePos
     */
    virtual const LPoint &rolePos() const override;

    /**
     * @brief Request to configure the surface size.
     *
     * This request is triggered each time the LLayerRole surface is mapped, when its exclusiveZone()
     * rect changes, or when the client sets one of the size() components to 0, indicating it expects the
     * compositor to set the size.
     * A configureSize() event must be sent to the client with the new suggested size.
     *
     * @note Clients are free to ignore the suggested size.
     *
     * The default implementation configures the surface using the client's size() hint
     * or the available exclusiveZone() space for components of size() that are 0, for
     * components where both opposite edges are set in anchor() or if anchor() contains
     * all edges or no edge at all.
     *
     * #### Default Implementation
     * @snippet LLayerRoleDefault.cpp configureRequest
     */
    virtual void configureRequest();

    /**
     * @brief Configure size.
     *
     * Asks the client to configure its surface with the specified size.\n
     * The size doesn't change immediately, see atomsChanged().
     *
     * @note This is just a suggestion, the client is free to ignore it.
     *
     * @param size The size of the surface in coordinates.
     */
    void configureSize(const LSize &size) noexcept;

    /**
     * @brief Notify property changes
     *
     * Notifies when one or more properties change.
     *
     * @see @ref Atoms
     *
     * @param changes Flags indicating which properties have changed.
     * @param prevAtoms The previous property values.
     *
     * #### Default Implementation
     * @snippet LLayerRoleDefault.cpp atomsChanged
     */
    virtual void atomsChanged(LBitset<AtomChanges> changes, const Atoms &prevAtoms);

    const Atoms &atoms() const noexcept
    {
        return m_atoms[m_currentAtomsIndex];
    }

    /**
     * @brief Surface size hint
     *
     * Returns the surface size hint set by the client to indicate the desired surface size during a configureRequest().\n
     * If one of the components is 0, it means the compositor should assign it.
     *
     * @note This is just a hint, to retrieve the actual surface size, use LSurface::size().
     *
     * @return The surface size hint.
     */
    const LSize &size() const noexcept
    {
        return atoms().size;
    }

    /**
     * @brief Get Anchored Edges
     *
     * Returns the flags containing the edges to which the surface is anchored.
     *
     * @return The anchored edges.
     */
    LBitset<LEdge> anchor() const noexcept
    {
        return atoms().anchor;
    }

    const LExclusiveZone &exclusiveZone() const noexcept
    {
        return m_exclusiveZone;
    }

    Int32 exclusiveZoneSize() const noexcept
    {
        return atoms().exclusiveZoneSize;
    }

    const LMargins &margins() const noexcept
    {
        return atoms().margins;
    }

    KeyboardInteractivity keyboardInteractivity() const noexcept
    {
        return atoms().keyboardInteractivity;
    }

    LEdge exclusiveEdge() const noexcept
    {
        return atoms().exclusiveEdge;
    }

    LSurfaceLayer layer() const noexcept
    {
        return atoms().layer;
    }

    LOutput *exclusiveOutput() const override
    {
        return m_exclusiveZone.output();
    }

    void setExclusiveOutput(LOutput *output) noexcept
    {
        m_exclusiveZone.setOutput(output);
    }

    const std::string &scope() noexcept
    {
        return m_scope;
    }

    void close() noexcept;

private:
    friend class Louvre::Protocols::LayerShell::RLayerSurface;

    enum Flags : UInt32
    {
        HasPendingSize                  = static_cast<UInt32>(1) << 0,
        HasPendingAnchor                = static_cast<UInt32>(1) << 1,
        HasPendingExclusiveZone         = static_cast<UInt32>(1) << 2,
        HasPendingMargin                = static_cast<UInt32>(1) << 3,
        HasPendingKeyboardInteractivity = static_cast<UInt32>(1) << 4,
        HasPendingLayer                 = static_cast<UInt32>(1) << 5,
        HasPendingExclusiveEdge         = static_cast<UInt32>(1) << 6,
        HasPendingInitialConf           = static_cast<UInt32>(1) << 7,
        HasConfToSend                   = static_cast<UInt32>(1) << 8,
        ClosedSent                      = static_cast<UInt32>(1) << 9,
    };

    Atoms &currentAtoms() noexcept
    {
        return m_atoms[m_currentAtomsIndex];
    }

    Atoms &pendingAtoms() noexcept
    {
        return m_atoms[1 - m_currentAtomsIndex];
    }

    LEdge edgesToSingleEdge() const noexcept;

    void handleSurfaceCommit(CommitOrigin origin) noexcept override;

    LExclusiveZone m_exclusiveZone { LEdgeNone, 0 };
    LBitset<Flags> m_flags { HasPendingInitialConf };
    Atoms m_atoms[2];
    UInt8 m_currentAtomsIndex { 0 };
    std::string m_scope;

    // Initial params
    LWeak<LOutput> m_initOutput;
    LSurfaceLayer m_initLayer;
};

#endif // LLAYERROLE_H
