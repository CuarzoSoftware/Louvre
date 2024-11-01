%global basever 2.10.0
%global origrel 1
%global somajor 2

Name:           cuarzo-louvre
Version:        %{basever}%{?origrel:_%{origrel}}
Release:        1%{?dist}
Summary:        C++ library for building Wayland compositors

License:        MIT
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
* Fri Nov 01 2024 Eduardo Hopperdietzel <ehopperdietzel@gmail.com> - %{basever}-%{origrel}
- Added LTexture::setFence() and LRenderBuffer::setFence() to prevent partial updates when used as source textures.
- Added LTexture::setDataFromGL() to enable wrapping of already created OpenGL textures.
- Replaced glFinish() calls with fences, ensuring synchronization without requiring CPU-side waiting.
- Fixed issue of black textures appearing on proprietary NVIDIA drivers (and potentially others). Thanks @renhiyama and @kingdomkind for all your help!
- Fixed synchronization issues between threads that were causing partial texture updates.
- Replaced calls to wl_client_destroy with wl_resource_post_error when an unknown buffer type is committed, which was causing a segmentation fault. Thanks @Ramblurr for reporting it!
- Fixed segfault occurring when an idle inhibitor resource was destroyed.
- Wayland Backend: Resolved issue where modifier keys were not released when the window lost focus. Thanks @renhiyama for reporting it!
- Added test to validate support for common texture formats and the synchronization of updates across threads.
- Updated SRM dependency to version >= v0.8.0.
