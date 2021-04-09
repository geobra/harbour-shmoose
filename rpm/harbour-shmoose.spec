Name: harbour-shmoose
Version: 0.8.0
Release:	1%{?dist}
Summary: Shmoose - XMPP Client for Sailfish OS

Group: Qt/Qt
License: GPL
URL: https://github.com/geobra/harbour-shmoose
Source0: %{name}-%{version}.tar.bz2
BuildRoot:	%(mktemp -ud %{_tmppath}/%{name}-%{version}-%{release}-XXXXXX)

BuildRequires:	pkgconfig(sailfishapp) >= 1.0.2
BuildRequires:  pkgconfig(Qt5Quick)
BuildRequires:  pkgconfig(Qt5Qml)
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  openssl-devel

%description
XMPP Client for Sailfish OS


%prep
%setup -q


%build
qmake CONFIG+=sailfishapp CONFIG+=sailfishapp_i18n DEFINES+=SFOS
make %{?_smp_mflags}


%install
rm -rf $RPM_BUILD_ROOT
# >> install pre
# << install pre
install -d %{buildroot}%{_bindir}
install -p -m 0755 %(pwd)/%{name} %{buildroot}%{_bindir}/%{name}
install -d %{buildroot}%{_datadir}/applications
install -d %{buildroot}%{_datadir}/lipstick/notificationcategories
install -d %{buildroot}%{_datadir}/%{name}
install -d %{buildroot}%{_datadir}/%{name}/qml
install -d %{buildroot}%{_datadir}/%{name}/icons
install -d %{buildroot}%{_datadir}/%{name}/translations
cp -Ra ./resources/qml/* %{buildroot}%{_datadir}/%{name}/qml
cp -Ra ./resources/icons/* %{buildroot}%{_datadir}/%{name}/icons
cp -Ra ./resources/translations/*.qm %{buildroot}%{_datadir}/%{name}/translations
install -d %{buildroot}%{_datadir}/icons/hicolor/86x86/apps
install -m 0444 -t %{buildroot}%{_datadir}/icons/hicolor/86x86/apps/ resources/icons/86x86/%{name}.png
install -p %(pwd)/resources/harbour-shmoose.desktop %{buildroot}%{_datadir}/applications/%{name}.desktop
install -p %(pwd)/resources/harbour-shmoose-message.conf %{buildroot}%{_datadir}/lipstick/notificationcategories/%{name}-message.conf

strip %{buildroot}%{_bindir}/%{name}

# >> install post
# << install post

desktop-file-install --delete-original \
    --dir %{buildroot}%{_datadir}/applications \
    %{buildroot}%{_datadir}/applications/*.desktop


%clean
rm -rf $RPM_BUILD_ROOT


%files
%defattr(-,root,root,-)
%{_datadir}/applications/%{name}.desktop
%{_datadir}/lipstick/notificationcategories/%{name}-message.conf
%{_datadir}/%{name}/qml
%{_datadir}/%{name}/icons
%{_datadir}/%{name}/translations
%{_datadir}/icons/hicolor/86x86/apps
%{_bindir}/%{name}
# >> files
# << files


%changelog

