%global basever 2.18.0
%global origrel 1
%global somajor 2

Name:           cuarzo-louvre
Version:        %{basever}%{?origrel:_%{origrel}}
Release:        1%{?dist}
Summary:        C++ library for building Wayland compositors

License:        LGPLv2.1
URL:            https://github.com/CuarzoSoftware/Louvre

BuildRequires:  tar
BuildRequires:  wget
BuildRequires:  meson
BuildRequires:  gcc-c++
BuildRequires:  pkgconfig(wayland-server)
BuildRequires:  pkgconfig(wayland-client)
BuildRequires:  pkgconfig(wayland-egl)
BuildRequires:  pkgconfig(gl)
BuildRequires:  pkgconfig(egl)
BuildRequires:  pkgconfig(glesv2)
BuildRequires:  pkgconfig(libudev)
BuildRequires:  pkgconfig(xcursor)
BuildRequires:  pkgconfig(xkbcommon)
BuildRequires:  pkgconfig(pixman-1)
BuildRequires:  pkgconfig(libdrm)
BuildRequires:  pkgconfig(libinput)
BuildRequires:  pkgconfig(libseat)
BuildRequires:  pkgconfig(SRM)

# Examples
BuildRequires:  pkgconfig(fontconfig)
BuildRequires:  pkgconfig(freetype2)
BuildRequires:  libicu-devel

%description
Louvre is a high-performance C++ library designed for building
Wayland compositors with a strong emphasis on ease of development.

%package        devel
Summary:        Development files for %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description    devel
The %{name}-devel package contains libraries and header files for
developing applications that use %{name}.

%package        examples
Summary:        Example applications using %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description    examples
The %{name}-examples package contains example applications using
%{name}.

%prep
rm -rf repo
rm -f src.tar.gz
mkdir -p repo
wget -O src.tar.gz %{url}/archive/refs/tags/v%{basever}-%{origrel}.tar.gz
tar --strip-components=1 -xzvf src.tar.gz -C repo

%build
pushd repo/src
%meson
%meson_build

%install
pushd repo/src
%meson_install

%files
%license repo/LICENSE
%doc repo/BUILD repo/CHANGES repo/VERSION
%{_libdir}/libLouvre.so.%{somajor}
%{_libdir}/Louvre/

%files examples
%{_usr}/local/share/wayland-sessions/louvre.desktop
%{_bindir}/louvre-default
%{_bindir}/louvre-views
%{_bindir}/louvre-weston-clone
%{_datadir}/Louvre/

%files devel
%doc repo/README.md repo/doxygen
%{_includedir}/Louvre/
%{_libdir}/libLouvre.so
%{_libdir}/pkgconfig/Louvre.pc

%changelog
* Sat May 10 2025 Eduardo Hopperdietzel <ehopperdietzel@gmail.com> - %{basever}-%{origrel}
- lvr-invisible-region: Allows clients to define invisible regions within their surfaces, enabling the compositor to skip rendering invisible pixels. See LSurface::invisibleRegion().
- lvr-svg-path: Provides an efficient way for clients to share paths with the compositor for masking or other purposes.
- lvr-background-blur: Allows clients to request background blurring for surfaces, with optional clipping masks. See LBackgroundBlur - thanks to @Fox2Code for contributing ideas and @renhiyama for testing and feedback.
- New LBackgroundBlur class to handle surface blur effects.
- New LRRect class for defining rounded rectangles.
- Added LCompositor::eventLoop() for quick access to the main wl_event_loop.
- LCompositor::addFdListener() and LCompositor::removeFdListener() are now deprecated, wl_event_loop_add_fd() should be used instead.
- Added LPointerScrollEvent::LegacyWheel source type for handling older, deprecated Libinput Wheel events.
- LPointerScrollEvent now includes hasX() and hasY() to indicate if values are provided for each axis.
- LPointerScrollEvent::axes120() has been replaced by discreteAxes().
- Removed additional event queues, only the main wl_display event queue is now used.
- Event queues are no longer disabled when the session is inactive. This sometimes caused a queue overflow and freeze upon session resumption - thanks @jgroboredo and @LeKinaSa for reporting and testing.
- Corrected wrong dispatching of discrete scroll events — thanks @jgroboredo and @LeKinaSa for reporting and testing.
- Fixed a bug in the default LPointer::pointerButtonEvent() implementation that prevented transient toplevels from gaining keyboard focus.
- Updated SRM dependency to version 0.12.1, which fixes issues related to hotplugging and screen flashing during session switching.
