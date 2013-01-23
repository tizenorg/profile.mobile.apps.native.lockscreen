%define PREFIX /usr/apps/org.tizen.lockscreen

Name:       org.tizen.lockscreen
Summary:    lockscreen application
Version:    0.1.7
Release:    1
Group:      TBD
License:    Apache
Source0:    %{name}-%{version}.tar.gz
BuildRequires: pkgconfig(capi-appfw-application)
BuildRequires: pkgconfig(ecore-x)
BuildRequires: pkgconfig(appcore-efl)
BuildRequires: pkgconfig(utilX)
BuildRequires: pkgconfig(ail)
BuildRequires: pkgconfig(notification)
BuildRequires: pkgconfig(security-server)
BuildRequires: pkgconfig(ui-gadget-1)
BuildRequires: pkgconfig(capi-system-info)

BuildRequires: cmake
BuildRequires: gettext
BuildRequires: edje-tools
%description
lockscreen application.

%prep
%setup -q

%build
LDFLAGS+="-Wl,--rpath=%{PREFIX}/lib -Wl,--as-needed";export LDFLAGS
cmake . -DCMAKE_INSTALL_PREFIX=%{PREFIX}
make %{?jobs:-j%jobs}

%install
%make_install
mkdir -p %{buildroot}/usr/apps/org.tizen.lockscreen/data
mkdir -p %{buildroot}/usr/share/license
install -m 0755 LICENSE.Flora %{buildroot}/usr/share/license/org.tizen.lockscreen

%post
/sbin/ldconfig

GOPTION="-g 6514"

%postun -p /sbin/ldconfig

%files
%manifest org.tizen.lockscreen.manifest
%defattr(-,root,root,-)
%attr(-,inhouse,inhouse) %dir /usr/apps/org.tizen.lockscreen/data
/usr/apps/org.tizen.lockscreen/*
/usr/share/packages/org.tizen.lockscreen.xml
/usr/share/license/org.tizen.lockscreen
