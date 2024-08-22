# 🎓 Tutorial {#tutorial_tmp}

This is a work in progress. In the meantime, you can get started by cloning a base template from the following repository:

https://github.com/CuarzoSoftware/LouvreTemplate

It is well-documented and already implements the scene and views system for efficient rendering and basic server-side decorations.

> The old tutorial can be found [here](https://github.com/CuarzoSoftware/Louvre/tree/main/doxygen/md/tutorial), however, it is outdated, and many concepts and APIs have changed since then.

## Key Concepts

Here are some links to the C++ API documentation to help you navigate it:

- Compositor [Initialization](@ref Louvre::LCompositor::initialized) and [Uninitialization](@ref Louvre::LCompositor::uninitialized).
- [Object Factory](@ref Factory).
- [Rendering Content to Displays.](@ref loutput_detailed)
- [Multithreading.](@ref render_multithreading)
- [Scene](@ref lscene_detailed) and [Views](@ref lview_detailed).
- [Fractional Scaling and Oversampling.](@ref Scaling)
- [Gamma Correction.](@ref lgammatable_detailed)
- [Tearing Control.](@ref VSync)
- [Pointer Events and Gestures.](@ref lpointer_detailed)
- [Pointer Constraints.](@ref pointer_constraints)
- [Cursor.](@ref lcursor_detailed)
- [Keyboard Events.](@ref lkeyboard_detailed)
- [Touch Events.](@ref ltouch_detailed)
- [Configuring Input Devices.](@ref linputdevice_detailed)
- [Screencasting.](@ref lscreenshotrequest_detailed)
- [Clipboard](@ref lclipboard_detailed) and [Drag & Drop.](@ref ldnd_detailed)
- [Surfaces and Roles.](@ref lsurface_detailed)
- [Session Lock Manager](@ref lsessionlockmanager_detailed) and [role.](@ref lsessionlockrole_detailed)
- [Toplevel](@ref ltoplevelrole_detailed) and [Popup](@ref ldnd_detailed) surfaces.
- [Wlr Layer Shell](@ref llayerrole_detailed) and [Exclusive Zones.](@ref lexclusivezone_detailed)
- [XDG Activation Tokens.](@ref lactivationtokenmanager_detailed)
- [Foreign Toplevel Window Management.](@ref lforeigntoplevelcontroller_detailed)
- [Foreign Toplevel List.](@ref Louvre::LToplevelRole::foreignHandleFilter)
- [Idle Listeners and Inhibitors.](@ref lidlelistener_detailed)