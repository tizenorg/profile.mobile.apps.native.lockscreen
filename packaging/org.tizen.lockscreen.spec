%bcond_with wayland

%define AppInstallPath /usr/apps/org.tizen.lockscreen
%define Exec lockscreen

Name:       org.tizen.lockscreen
Summary:    Lockscreen app
Version:    0.0.1
Release:    1
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz

%if "%{?tizen_profile_name}" == "wearable"
ExcludeArch: %{arm} %ix86 x86_64
%endif

%if "%{?tizen_profile_name}"=="tv"
ExcludeArch: %{arm} %ix86 x86_64
%endif

BuildRequires:  pkgconfig(appcore-efl)
BuildRequires:  pkgconfig(ecore)
BuildRequires:  pkgconfig(eina)
BuildRequires:  pkgconfig(evas)
BuildRequires:  pkgconfig(bundle)
BuildRequires:  pkgconfig(capi-system-sensor)
BuildRequires:  pkgconfig(capi-system-info)
BuildRequires:	pkgconfig(capi-system-runtime-info)
BuildRequires:  pkgconfig(capi-system-system-settings)
BuildRequires:  pkgconfig(capi-system-device)
BuildRequires:  pkgconfig(capi-appfw-application)
BuildRequires:  pkgconfig(capi-appfw-preference)
BuildRequires:	pkgconfig(capi-appfw-app-manager)
BuildRequires:  pkgconfig(deviced)
BuildRequires:  pkgconfig(edje)
BuildRequires:  pkgconfig(aul)
BuildRequires:  pkgconfig(ail)
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(dbus-1)
BuildRequires:  pkgconfig(dbus-glib-1)
BuildRequires:  pkgconfig(elementary)
BuildRequires:  pkgconfig(notification)
BuildRequires:  pkgconfig(minicontrol-viewer)
BuildRequires:  pkgconfig(security-server)
BuildRequires:  pkgconfig(minicontrol-monitor)
BuildRequires:  pkgconfig(ui-gadget-1)
BuildRequires:  pkgconfig(feedback)
BuildRequires:  pkgconfig(msg-service)
BuildRequires:  pkgconfig(tapi)
BuildRequires:  pkgconfig(efl-extension)
BuildRequires:  pkgconfig(key-manager)
BuildRequires:  pkgconfig(accounts-svc)
BuildRequires:  cmake
BuildRequires:  edje-tools
BuildRequires:  gettext-tools
#BuildRequires:  model-build-features

%if %{with wayland}
BuildRequires:  pkgconfig(ecore-wayland)
%else
BuildRequires:  pkgconfig(ecore-x)
BuildRequires:  pkgconfig(utilX)
%endif

%description
Lockscreen application for Tizen.

%prep
%setup -q

%build
%if 0%{?tizen_build_binary_release_type_eng}
export CFLAGS="$CFLAGS -DTIZEN_ENGINEER_MODE"
export CXXFLAGS="$CXXFLAGS -DTIZEN_ENGINEER_MODE"
export FFLAGS="$FFLAGS -DTIZEN_ENGINEER_MODE"
%endif

%ifarch %{arm}
export CFLAGS="$CFLAGS -DTIZEN_BUILD_TARGET"
export CXXFLAGS="$CXXFLAGS -DTIZEN_BUILD_TARGET"
export FFLAGS="$FFLAGS -DTIZEN_BUILD_TARGET"
%else
export CFLAGS="$CFLAGS -DTIZEN_BUILD_EMULATOR"
export CXXFLAGS="$CXXFLAGS -DTIZEN_BUILD_EMULATOR"
export FFLAGS="$FFLAGS -DTIZEN_BUILD_EMULATOR"
%endif

%if %{with wayland}
export WAYLAND_SUPPORT=On
export X11_SUPPORT=Off
%else
export WAYLAND_SUPPORT=Off
export X11_SUPPORT=On
%endif

cmake . -DCMAKE_INSTALL_PREFIX="%{AppInstallPath}" -DCMAKE_TARGET="%{Exec}" -DCMAKE_PACKAGE="%{name}" -DWAYLAND_SUPPORT=${WAYLAND_SUPPORT} -DX11_SUPPORT=${X11_SUPPORT}
make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install

%post
/sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%manifest org.tizen.lockscreen.manifest
%defattr(-,root,root,-)
%{AppInstallPath}/bin/lockscreen
%{AppInstallPath}/res/images/*.png
%{AppInstallPath}/res/edje/*.edj
%{AppInstallPath}/res/locale/*/LC_MESSAGES/*
/usr/share/packages/org.tizen.lockscreen.xml
