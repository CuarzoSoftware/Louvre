%global basever 2.16.2
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
* Sun Mar 30 2025 Eduardo Hopperdietzel <ehopperdietzel@gmail.com> - %{basever}-%{origrel}
- LToplevelRole::startMoveRequest() and LToplevelRole::startResizeRequest() now ignore requests triggered by pointer button events if the button isn't currently pressed.
- Renamed LPointer::pressedKeys() to pressedButtons().
- Removed the assumption that toplevels will be unmapped after sending a close() event.
- louvre-views: Fixed a bug causing fullscreen animations to show black on some clients.
- louvre-views: Display fadeout animation of toplevels not only during the first unmap.
