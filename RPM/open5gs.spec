%global _build_id_links none
%global _hardened_build 1

# basesuffix identifies your packaging (always appended to Release).
%global basesuffix .sslconsult
%global releaseNum 21

Name:           open5gs
Version:        2.7.7
Release:        %{releaseNum}%{basesuffix}%{?dist}
Summary:        Open Source Core Network for 5G

License:        AGPL-3.0-or-later
URL:            https://open5gs.org
Source0:        https://github.com/open5gs/open5gs/archive/refs/tags/v%{version}.tar.gz#/open5gs-v%{version}.tar.gz

BuildRequires:  gcc
BuildRequires:  gcc-c++
BuildRequires:  meson
BuildRequires:  ninja-build
BuildRequires:  cmake
BuildRequires:  flex
BuildRequires:  bison
BuildRequires:  git
BuildRequires:  systemd-devel
BuildRequires:  pkgconfig(libsctp)
BuildRequires:  pkgconfig(libidn)
BuildRequires:  pkgconfig(gnutls)
BuildRequires:  pkgconfig(libgcrypt)
BuildRequires:  pkgconfig(openssl)
BuildRequires:  pkgconfig(libsasl2)
BuildRequires:  pkgconfig(yaml-0.1)
BuildRequires:  pkgconfig(libmongoc-1.0)
BuildRequires:  pkgconfig(libmicrohttpd)
BuildRequires:  pkgconfig(libcurl)
BuildRequires:  pkgconfig(libnghttp2)
BuildRequires:  pkgconfig(talloc)
BuildRequires:  pkgconfig(libtins)
BuildRequires:  pkgconfig(usrsctp)
BuildRequires:  chrpath

# NOTE: mongodb-org-server is a runtime prerequisite, not a package dependency.
# Users should install it manually from the official MongoDB repository.

%description
Open5GS is an open source implementation of the 5G Core and EPC,
i.e., the core network of LTE/NR network (Release-17).
This is a meta-package and does not install any files.

# --- Sub-package Definitions ---

%package common
Summary:        Common files for the Open5GS Project
Requires:       lksctp-tools

%description common
This package contains the common libraries, configuration, and directories shared by all Open5GS services. It creates the 'open5gs' user and group.

%package amf
Summary:        Open5GS Access and Mobility Management Function (AMF)
Requires:       %{name}-common = %{version}-%{release}
Requires(post):   systemd
Requires(preun):  systemd
Requires(postun): systemd

%description amf
This package provides the Open5GS Access and Mobility Management Function (AMF).

%package ausf
Summary:        Open5GS Authentication Server Function (AUSF)
Requires:       %{name}-common = %{version}-%{release}
Requires(post):   systemd
Requires(preun):  systemd
Requires(postun): systemd

%description ausf
This package provides the Open5GS Authentication Server Function (AUSF).

%package bsf
Summary:        Open5GS Binding Support Function (BSF)
Requires:       %{name}-common = %{version}-%{release}
Requires(post):   systemd
Requires(preun):  systemd
Requires(postun): systemd

%description bsf
This package provides the Open5GS Binding Support Function (BSF).

%package hss
Summary:        Open5GS Home Subscriber Server (HSS)
Requires:       %{name}-common = %{version}-%{release}
Requires(post):   systemd
Requires(preun):  systemd
Requires(postun): systemd

%description hss
This package provides the Open5GS Home Subscriber Server (HSS).

%package mme
Summary:        Open5GS Mobility Management Entity (MME)
Requires:       %{name}-common = %{version}-%{release}
Requires(post):   systemd
Requires(preun):  systemd
Requires(postun): systemd

%description mme
This package provides the Open5GS Mobility Management Entity (MME).

%package nrf
Summary:        Open5GS Network Repository Function (NRF)
Requires:       %{name}-common = %{version}-%{release}
Requires(post):   systemd
Requires(preun):  systemd
Requires(postun): systemd

%description nrf
This package provides the Open5GS Network Repository Function (NRF).

%package nssf
Summary:        Open5GS Network Slice Selection Function (NSSF)
Requires:       %{name}-common = %{version}-%{release}
Requires(post):   systemd
Requires(preun):  systemd
Requires(postun): systemd

%description nssf
This package provides the Open5GS Network Slice Selection Function (NSSF).

%package pcf
Summary:        Open5GS Policy Control Function (PCF)
Requires:       %{name}-common = %{version}-%{release}
Requires(post):   systemd
Requires(preun):  systemd
Requires(postun): systemd

%description pcf
This package provides the Open5GS Policy Control Function (PCF).

%package pcrf
Summary:        Open5GS Policy and Charging Rules Function (PCRF)
Requires:       %{name}-common = %{version}-%{release}
Requires(post):   systemd
Requires(preun):  systemd
Requires(postun): systemd

%description pcrf
This package provides the Open5GS Policy and Charging Rules Function (PCRF).

%package scp
Summary:        Open5GS Service Communication Proxy (SCP)
Requires:       %{name}-common = %{version}-%{release}
Requires(post):   systemd
Requires(preun):  systemd
Requires(postun): systemd

%description scp
This package provides the Open5GS Service Communication Proxy (SCP).

%package sepp
Summary:        Open5GS Security Edge Protection Proxy (SEPP)
Requires:       %{name}-common = %{version}-%{release}
Requires(post):   systemd
Requires(preun):  systemd
Requires(postun): systemd

%description sepp
This package provides the Open5GS Security Edge Protection Proxy (SEPP).

%package sgwc
Summary:        Open5GS S-GW Control Plane (SGW-C)
Requires:       %{name}-common = %{version}-%{release}
Requires(post):   systemd
Requires(preun):  systemd
Requires(postun): systemd

%description sgwc
This package provides the Open5GS S-GW Control Plane function (SGW-C).

%package sgwu
Summary:        Open5GS S-GW User Plane (SGW-U)
Requires:       %{name}-common = %{version}-%{release}
Requires(post):   systemd
Requires(preun):  systemd
Requires(postun): systemd

%description sgwu
This package provides the Open5GS S-GW User Plane function (SGW-U).

%package smf
Summary:        Open5GS Session Management Function (SMF)
Requires:       %{name}-common = %{version}-%{release}
Requires(post):   systemd
Requires(preun):  systemd
Requires(postun): systemd

%description smf
This package provides the Open5GS Session Management Function (SMF).

%package udm
Summary:        Open5GS Unified Data Management (UDM)
Requires:       %{name}-common = %{version}-%{release}
Requires(post):   systemd
Requires(preun):  systemd
Requires(postun): systemd

%description udm
This package provides the Open5GS Unified Data Management (UDM).

%package udr
Summary:        Open5GS Unified Data Repository (UDR)
Requires:       %{name}-common = %{version}-%{release}
Requires(post):   systemd
Requires(preun):  systemd
Requires(postun): systemd

%description udr
This package provides the Open5GS Unified Data Repository (UDR).

%package upf
Summary:        Open5GS User Plane Function (UPF)
Requires:       %{name}-common = %{version}-%{release}
Requires(post):   systemd
Requires(post):   procps-ng
Requires(preun):  iproute
Requires(preun):  systemd
Requires(postun): systemd

%description upf
This package provides the Open5GS User Plane Function (UPF).


%prep
%autosetup -p1
meson subprojects download

%build
%meson \
    --prefix=%{_prefix} \
    --sysconfdir=%{_sysconfdir} \
    --localstatedir=%{_localstatedir} \
    --libdir=%{_libdir}
%meson_build

%install
%meson_install

# Create directories
install -d -m 0755 %{buildroot}%{_localstatedir}/log/%{name}/tls
install -d -m 0755 %{buildroot}%{_localstatedir}/lib/open5gs
install -d -m 0755 %{buildroot}%{_sysconfdir}/open5gs/tls
install -d -m 0755 %{buildroot}%{_sysconfdir}/open5gs/hnet
install -d -m 0755 %{buildroot}%{_sysconfdir}/freeDiameter
install -d -m 0755 %{buildroot}%{_sysconfdir}/logrotate.d
install -d -m 0755 %{buildroot}%{_sysconfdir}/systemd/network
install -d -m 0755 %{buildroot}%{_unitdir}
install -d -m 0755 %{buildroot}%{_libdir}
install -d -m 0755 %{buildroot}%{_sysctldir}
echo "net.ipv6.conf.all.disable_ipv6=0" > %{buildroot}%{_sysctldir}/30-open5gs-upf.conf

# Copy all config files to their final destinations in buildroot
# The %files lists will sort them into the correct packages.
# NOTE: Using '|| true' to prevent build failure if a directory is empty.
cp -a %{_builddir}/%{name}-%{version}/redhat-linux-build/configs/open5gs/*.yaml %{buildroot}%{_sysconfdir}/open5gs/ || true
cp -a %{_builddir}/%{name}-%{version}/redhat-linux-build/configs/open5gs/tls/* %{buildroot}%{_sysconfdir}/open5gs/tls/ || true
cp -a %{_builddir}/%{name}-%{version}/redhat-linux-build/configs/open5gs/hnet/* %{buildroot}%{_sysconfdir}/open5gs/hnet/ || true
cp -a %{_builddir}/%{name}-%{version}/redhat-linux-build/configs/freeDiameter/* %{buildroot}%{_sysconfdir}/freeDiameter/ || true
cp -a %{_builddir}/%{name}-%{version}/redhat-linux-build/configs/logrotate/open5gs %{buildroot}%{_sysconfdir}/logrotate.d/ || true
cp -a %{_builddir}/%{name}-%{version}/redhat-linux-build/configs/systemd/*.service %{buildroot}%{_unitdir}/ || true
cp -a %{_builddir}/%{name}-%{version}/redhat-linux-build/configs/systemd/99-open5gs.net* %{buildroot}%{_sysconfdir}/systemd/network/ || true


# Copy shared libraries
find %{_builddir}/%{name}-%{version}/redhat-linux-build/lib -type f -name '*.so*' -exec cp -a '{}' %{buildroot}%{_libdir}/ ';'
# Copy subproject shared libraries (prometheus-client-c, freeDiameter)
find %{_builddir}/%{name}-%{version}/redhat-linux-build/subprojects -type f -name '*.so*' -exec cp -a '{}' %{buildroot}%{_libdir}/ ';'

# Fix RPATH
find %{buildroot}%{_libdir} -type f -name '*.so*' -exec file '{}' \; | grep 'ELF' | awk -F: '{print $1}' | xargs -r chrpath --delete
find %{buildroot}%{_bindir} -type f -exec file '{}' \; | grep 'ELF' | awk -F: '{print $1}' | xargs -r chrpath --delete

%check
%meson_test --no-suite integration || true

# --- Systemd Scriptlets ---
%post amf
%systemd_post open5gs-amfd.service
%preun amf
%systemd_preun open5gs-amfd.service
%postun amf
%systemd_postun_with_restart open5gs-amfd.service

%post ausf
%systemd_post open5gs-ausfd.service
%preun ausf
%systemd_preun open5gs-ausfd.service
%postun ausf
%systemd_postun_with_restart open5gs-ausfd.service

%post bsf
%systemd_post open5gs-bsfd.service
%preun bsf
%systemd_preun open5gs-bsfd.service
%postun bsf
%systemd_postun_with_restart open5gs-bsfd.service

%post hss
%systemd_post open5gs-hssd.service
%preun hss
%systemd_preun open5gs-hssd.service
%postun hss
%systemd_postun_with_restart open5gs-hssd.service

%post mme
%systemd_post open5gs-mmed.service
%preun mme
%systemd_preun open5gs-mmed.service
%postun mme
%systemd_postun_with_restart open5gs-mmed.service

%post nrf
%systemd_post open5gs-nrfd.service
%preun nrf
%systemd_preun open5gs-nrfd.service
%postun nrf
%systemd_postun_with_restart open5gs-nrfd.service

%post nssf
%systemd_post open5gs-nssfd.service
%preun nssf
%systemd_preun open5gs-nssfd.service
%postun nssf
%systemd_postun_with_restart open5gs-nssfd.service

%post pcf
%systemd_post open5gs-pcfd.service
%preun pcf
%systemd_preun open5gs-pcfd.service
%postun pcf
%systemd_postun_with_restart open5gs-pcfd.service

%post pcrf
%systemd_post open5gs-pcrfd.service
%preun pcrf
%systemd_preun open5gs-pcrfd.service
%postun pcrf
%systemd_postun_with_restart open5gs-pcrfd.service

%post scp
%systemd_post open5gs-scpd.service
%preun scp
%systemd_preun open5gs-scpd.service
%postun scp
%systemd_postun_with_restart open5gs-scpd.service

%post sepp
%systemd_post open5gs-seppd.service
%preun sepp
%systemd_preun open5gs-seppd.service
%postun sepp
%systemd_postun_with_restart open5gs-seppd.service

%post sgwc
%systemd_post open5gs-sgwcd.service
%preun sgwc
%systemd_preun open5gs-sgwcd.service
%postun sgwc
%systemd_postun_with_restart open5gs-sgwcd.service

%post sgwu
%systemd_post open5gs-sgwud.service
%preun sgwu
%systemd_preun open5gs-sgwud.service
%postun sgwu
%systemd_postun_with_restart open5gs-sgwud.service

%post smf
%systemd_post open5gs-smfd.service
%preun smf
%systemd_preun open5gs-smfd.service
%postun smf
%systemd_postun_with_restart open5gs-smfd.service

%post udm
%systemd_post open5gs-udmd.service
%preun udm
%systemd_preun open5gs-udmd.service
%postun udm
%systemd_postun_with_restart open5gs-udmd.service

%post udr
%systemd_post open5gs-udrd.service
%preun udr
%systemd_preun open5gs-udrd.service
%postun udr
%systemd_postun_with_restart open5gs-udrd.service

%post upf
%systemd_post open5gs-upfd.service
if [ $1 -eq 1 ]; then
    # --- UPF Network Configuration ---
    # The UPF requires systemd-networkd for its .network file and may require IPv6.

    # 1. Enable systemd-networkd if it is not already enabled.
    if ! systemctl is-enabled -q systemd-networkd; then
        systemctl enable systemd-networkd >/dev/null 2>&1 || :
    fi

    # 2. Restart systemd-networkd to apply the 99-open5gs.network configuration.
    if [ -d "/run/systemd" ]; then
        systemctl restart systemd-networkd >/dev/null 2>&1 || :
    fi

    # 3. Enable IPv6 if it's found to be disabled system-wide.
    # Apply the new sysctl setting immediately
    sysctl -p %{_sysctldir}/30-open5gs-upf.conf >/dev/null 2>&1 || :
fi
%preun upf
%systemd_preun open5gs-upfd.service
if [ $1 -eq 0 ]; then
    # Cleanly remove the ogstun network interface if it exists.
    if grep -q "ogstun" /proc/net/dev; then
        ip tuntap del name ogstun mode tun
    fi
fi
%postun upf
%systemd_postun_with_restart open5gs-upfd.service

%pre common
getent group open5gs >/dev/null || groupadd -r open5gs
getent passwd open5gs >/dev/null || \
    useradd -r -g open5gs -d %{_localstatedir}/lib/open5gs \
    -s /sbin/nologin -c "Open5GS Service Account" open5gs
exit 0

%preun common
if [ $1 -eq 0 ]; then
    userdel open5gs 2>/dev/null || :
    groupdel open5gs 2>/dev/null || :
fi

# --- File Lists ---

%files
# This main package is a meta-package and owns no files.

%files common
%license LICENSE
%doc README.md
%exclude %{_libdir}/*.symbols
%exclude %{_libdir}/freeDiameter/*.fdx-*.debug
# Shared libraries and extensions
%{_libdir}/libogs*.so*
%{_libdir}/libfd*.so*
%{_libdir}/lib*prom*.so*
%{_libdir}/freeDiameter/*.fdx
# Database provisioning tool (shared by HSS/EPC and UDR/5GC deployments)
%{_bindir}/open5gs-dbctl
# Common config files
%config(noreplace) %{_sysconfdir}/open5gs/tls/ca.crt
%config(noreplace) %{_sysconfdir}/logrotate.d/open5gs
# Owned directories
%dir %{_sysconfdir}/open5gs
%dir %{_sysconfdir}/open5gs/tls
%dir %{_sysconfdir}/open5gs/hnet
%dir %{_sysconfdir}/freeDiameter
%dir %attr(0755, open5gs, open5gs) %{_localstatedir}/log/%{name}
%dir %attr(0755, open5gs, open5gs) %{_localstatedir}/log/%{name}/tls
%dir %attr(0755, open5gs, open5gs) %{_localstatedir}/lib/open5gs

%files amf
%{_bindir}/open5gs-amfd
%config(noreplace) %{_sysconfdir}/open5gs/amf.yaml
%config(noreplace) %{_sysconfdir}/open5gs/tls/amf.key
%config(noreplace) %{_sysconfdir}/open5gs/tls/amf.crt
%{_unitdir}/open5gs-amfd.service

%files ausf
%{_bindir}/open5gs-ausfd
%config(noreplace) %{_sysconfdir}/open5gs/ausf.yaml
%config(noreplace) %{_sysconfdir}/open5gs/tls/ausf.key
%config(noreplace) %{_sysconfdir}/open5gs/tls/ausf.crt
%{_unitdir}/open5gs-ausfd.service

%files bsf
%{_bindir}/open5gs-bsfd
%config(noreplace) %{_sysconfdir}/open5gs/bsf.yaml
%config(noreplace) %{_sysconfdir}/open5gs/tls/bsf.key
%config(noreplace) %{_sysconfdir}/open5gs/tls/bsf.crt
%{_unitdir}/open5gs-bsfd.service

%files hss
%{_bindir}/open5gs-hssd
%config(noreplace) %{_sysconfdir}/freeDiameter/hss.*
%config(noreplace) %{_sysconfdir}/open5gs/hss.yaml
%config(noreplace) %{_sysconfdir}/open5gs/tls/hss.key
%config(noreplace) %{_sysconfdir}/open5gs/tls/hss.crt
%{_unitdir}/open5gs-hssd.service

%files mme
%{_bindir}/open5gs-mmed
%config(noreplace) %{_sysconfdir}/freeDiameter/mme.*
%config(noreplace) %{_sysconfdir}/open5gs/mme.yaml
%config(noreplace) %{_sysconfdir}/open5gs/tls/mme.key
%config(noreplace) %{_sysconfdir}/open5gs/tls/mme.crt
%{_unitdir}/open5gs-mmed.service

%files nrf
%{_bindir}/open5gs-nrfd
%config(noreplace) %{_sysconfdir}/open5gs/nrf.yaml
%config(noreplace) %{_sysconfdir}/open5gs/tls/nrf.key
%config(noreplace) %{_sysconfdir}/open5gs/tls/nrf.crt
%{_unitdir}/open5gs-nrfd.service

%files nssf
%{_bindir}/open5gs-nssfd
%config(noreplace) %{_sysconfdir}/open5gs/nssf.yaml
%config(noreplace) %{_sysconfdir}/open5gs/tls/nssf.key
%config(noreplace) %{_sysconfdir}/open5gs/tls/nssf.crt
%{_unitdir}/open5gs-nssfd.service

%files pcf
%{_bindir}/open5gs-pcfd
%config(noreplace) %{_sysconfdir}/open5gs/pcf.yaml
%config(noreplace) %{_sysconfdir}/open5gs/tls/pcf.key
%config(noreplace) %{_sysconfdir}/open5gs/tls/pcf.crt
%{_unitdir}/open5gs-pcfd.service

%files pcrf
%{_bindir}/open5gs-pcrfd
%config(noreplace) %{_sysconfdir}/freeDiameter/pcrf.*
%config(noreplace) %{_sysconfdir}/open5gs/pcrf.yaml
%config(noreplace) %{_sysconfdir}/open5gs/tls/pcrf.key
%config(noreplace) %{_sysconfdir}/open5gs/tls/pcrf.crt
%{_unitdir}/open5gs-pcrfd.service

%files scp
%{_bindir}/open5gs-scpd
%config(noreplace) %{_sysconfdir}/open5gs/scp.yaml
%config(noreplace) %{_sysconfdir}/open5gs/tls/scp.key
%config(noreplace) %{_sysconfdir}/open5gs/tls/scp.crt
%{_unitdir}/open5gs-scpd.service

%files sepp
%{_bindir}/open5gs-seppd
%config(noreplace) %{_sysconfdir}/open5gs/sepp*.yaml
%config(noreplace) %{_sysconfdir}/open5gs/tls/sepp*.key
%config(noreplace) %{_sysconfdir}/open5gs/tls/sepp*.crt
%{_unitdir}/open5gs-seppd.service

%files sgwc
%{_bindir}/open5gs-sgwcd
%config(noreplace) %{_sysconfdir}/open5gs/sgwc.yaml
%{_unitdir}/open5gs-sgwcd.service

%files sgwu
%{_bindir}/open5gs-sgwud
%config(noreplace) %{_sysconfdir}/open5gs/sgwu.yaml
%{_unitdir}/open5gs-sgwud.service

%files smf
%{_bindir}/open5gs-smfd
%config(noreplace) %{_sysconfdir}/freeDiameter/smf.*
%config(noreplace) %{_sysconfdir}/open5gs/smf.yaml
%config(noreplace) %{_sysconfdir}/open5gs/tls/smf.key
%config(noreplace) %{_sysconfdir}/open5gs/tls/smf.crt
%{_unitdir}/open5gs-smfd.service

%files udm
%{_bindir}/open5gs-udmd
%config(noreplace) %{_sysconfdir}/open5gs/udm.yaml
%config(noreplace) %{_sysconfdir}/open5gs/tls/udm.key
%config(noreplace) %{_sysconfdir}/open5gs/tls/udm.crt
%config(noreplace) %{_sysconfdir}/open5gs/hnet/curve25519-*.key
%config(noreplace) %{_sysconfdir}/open5gs/hnet/secp256r1-*.key
%{_unitdir}/open5gs-udmd.service

%files udr
%{_bindir}/open5gs-udrd
%config(noreplace) %{_sysconfdir}/open5gs/udr.yaml
%config(noreplace) %{_sysconfdir}/open5gs/tls/udr.key
%config(noreplace) %{_sysconfdir}/open5gs/tls/udr.crt
%{_unitdir}/open5gs-udrd.service

%files upf
%{_bindir}/open5gs-upfd
%config(noreplace) %{_sysconfdir}/open5gs/upf.yaml
%config(noreplace) %{_sysconfdir}/systemd/network/99-open5gs.net*
%config(noreplace) %{_sysctldir}/30-open5gs-upf.conf
%{_unitdir}/open5gs-upfd.service

%changelog
* Tue Jul 28 2026 Keith Milner <kamilner@sslconsult.com> - 2.7.7-21
- packaging: install open5gs-dbctl and ship it in the common sub-package;
  the misc/db meson build previously only ran configure_file on the script
  without an install rule, so the database provisioning tool was never
  packaged. Now installed to %{_bindir}/open5gs-dbctl (mode 0755) and owned
  by open5gs-common so it is available to both EPC (HSS) and 5GC (UDR/UDM)
  deployments. Note: requires the MongoDB shell (mongosh) at runtime.
* Tue Jun 30 2026 Keith Milner <kamilner@sslconsult.com> - 2.7.7-20
- smf: fix build break in gsm-sm.c gtp_cause_from_diameter(); a duplicated
  OGS_DIAM_UNABLE_TO_DELIVER case had been left outside the inner
  switch(dia_err), landing as a label on switch(gtp_version) (uint8_t) and
  failing the build with -Werror=switch-outside-range and
  -Werror=implicit-fallthrough. Release 19 did not compile.
* Tue May 05 2026 Keith Milner <kamilner@sslconsult.com> - 2.7.7-19
- smf: force gy_enabled when ctf=yes regardless of PCRF Online AVP;
  open5gs PCRF never sets Online: 1 in CCA so Gy CCR was never sent
  even with smf.ctf.enabled: yes configured.
* Fri May 01 2026 Keith Milner <kamilner@sslconsult.com> - 2.7.7-18
- sgwc: add Gz offline charging CDR output (Phase 1); each completed
  session produces one JSON record (a single line) appended to a
  configurable file (default /var/log/open5gs/sgwc-cdrs.jsonl).
  Fields follow 3GPP TS 32.298 S-CDR layout: IMSI, MSISDN, APN,
  PDN type, UE IP, SGW/PGW address, charging ID, RAT type, serving
  network (MCC/MNC), charging characteristics, open/close timestamps,
  duration, cause, and optional ULI (TAI + ECGI). Configure via
  sgwc.gz.cdr_file in sgwc.yaml. Architecture preserves a clean
  data-capture layer for a future GTP'/ASN.1 BER transport phase.
* Thu Apr 30 2026 Keith Milner <kamilner@sslconsult.com> - 2.7.7-17
- smf: add absolute session timeout; new session_timeout key (seconds)
  under smf: in smf.yaml terminates sessions that remain active longer
  than the configured lifetime. Per-session ogs_timer_t fires
  SMF_EVT_SESSION_RELEASE with a new OGS_PFCP_DELETE_TRIGGER_SESSION_TIMEOUT
  trigger; 5GC sessions follow the normal network-requested PDU Session
  Release path (N1/N2 command to UE); EPC sessions delete the PFCP
  session then run Gx/Gy/S6b CCR-Termination. Disabled when unset or 0.
* Thu Apr 02 2026 Keith Milner <kamilner@sslconsult.com> - 2.7.7-16
- smf: detect UPF restart via Recovery Timestamp in Association Setup
  Request/Response handlers; triggers pfcp_restoration() when remote
  timestamp increases, mirroring the existing heartbeat handler logic.
- smf: handle UPF heartbeat timeout for EPC sessions; sends Delete
  Bearer Request toward SGW (GTPv2, fire-and-forget) then runs
  Gx/Gy/S6b CCR-Termination via smf_gsm_state_wait_epc_auth_release,
  replacing the previous "EPC restoration is not implemented" stub.
* Wed Apr 01 2026 Keith Milner <kamilner@sslconsult.com> - 2.7.7-15
- smf: graceful PFCP session teardown on SIGTERM; iterates all active
  sessions and sends PFCP Session Deletion Requests to the UPF before
  exiting, then sends PFCP Association Release Requests to each UPF peer.
  Holds the event loop open for up to 3 s to receive responses. Prevents
  stale GTP-U TEIDs and Error Indication storms on SGW-U after SMF restart.
- upf: handle PFCP Association Release Request from SMF; send Association
  Release Response and transition FSM back to will_associate state.
* Wed Apr 01 2026 Keith Milner <kamilner@sslconsult.com> - 2.7.7-14
- upf: implements PFCP graceful restart message to support
  smf graceful shutdown/restart
* Wed Apr 01 2026 Keith Milner <kamilner@sslconsult.com> - 2.7.7-13
- smf: Add minimal support for PCO option 0x1a. This is defined as
  "IP address allocation via NAS signalling only" which is the default
  capability on Open5GS anyway. In this case, the SMF just needs to accept
  this message, and does not need to return any response. Normally, no
  other action is required.
* Tue Mar 31 2026 Keith Milner <kamilner@sslconsult.com> - 2.7.7-12
- smf: respond to PCO container IDs 0x0023 (PDU Session ID) and 0x0024
  (EPS Bearer Identity) instead of logging unknown-PCO warnings; 0x0023
  returns the session's 5G PSI if present (4G/5G interworking) or 0x00
  for pure-4G deployments; 0x0024 returns the default bearer EBI for
  5G-to-4G handover correlation; both per 3GPP TS 24.301 §10.5.6.3

* Sun Mar 29 2026 Keith Milner <kamilner@sslconsult.com> - 2.7.7-11
- mme: fix Create Session Response rejection handling; MME was checking
  for PGW S5C TEID and PAA before checking the cause value, so a
  forwarded PGW rejection (e.g. Cause:125 no PCRF) was misclassified
  as Cause:103 (Conditional IE missing); now checks cause first and
  jumps to failure path for any non-accepted cause, matching the same
  fix applied to SGW-C in 2.7.7-10

* Sun Mar 29 2026 Keith Milner <kamilner@sslconsult.com> - 2.7.7-10
- sgwc: fix Create Session Response rejection forwarding; when PGW
  rejects CSR the SGW-C was checking for mandatory IEs before checking
  the cause, overriding the real PGW cause with 103 (Conditional IE
  missing) and sending a response without the SGW S11 TEID, causing
  MME to fail with "No S11 TEID"; now forwards PGW cause correctly
  with SGW S11 TEID included per TS 29.274 §7.2.2
- smf: map Diameter Result-Code 3002 (UNABLE_TO_DELIVER) to GTP cause
  REMOTE_PEER_NOT_RESPONDING (52) instead of UE_NOT_AUTHORISED

* Sat Mar 28 2026 Keith Milner <kamilner@sslconsult.com> - 2.7.7-9
- sgwc: fix crash in multi-PDN Modify Bearer forwarding; s5c_xact had
  assoc_xact_id set to s11_xact->id but s11_xact->assoc_xact_id was
  not set, causing ogs_gtp_xact_deassociate() assertion failure; fix
  by leaving assoc_xact_id unset and looking up S11 xact via UE
  context mb_s11_xact_id in the response handler

* Sat Mar 28 2026 Keith Milner <kamilner@sslconsult.com> - 2.7.7-8
- sgwc: support multi-PDN Modify Bearer forwarding to distinct PGWs (e.g.
  IMS/VoLTE + data over S8); add mb_pgw_pending counter to sgwc_ue so
  MBResponse is sent to MME only after all S5C responses are received

* Sat Mar 28 2026 Keith Milner <kamilner@sslconsult.com> - 2.7.7-7
- sgwc: fix Modify Bearer Request to PGW: use s5_s8_u_sgw_f_teid (TLV
  instance 2) instead of s4_u_sgsn_f_teid (instance 1) in the bearer
  context; instance 1 is the SGSN→SGW direction; PGW parsers expect
  the SGW-U S5/S8 GTP-U TEID at the same instance as in Create Session

* Sat Mar 28 2026 Keith Milner <kamilner@sslconsult.com> - 2.7.7-6
- sgwc: forward Modify Bearer Request to PGW over S5/S8 for home-routed
  roaming; without this the PGW-U never received the SGW-U GTP-U endpoint,
  permanently black-holing downlink traffic after initial attach (TS 29.274
  §7.2.7)

* Sat Mar 28 2026 Keith Milner <kamilner@sslconsult.com> - 2.7.7-5
- smf: fix PRIu64/uint32_t format mismatch in APN-AMBR debug log

* Tue Mar 24 2026 Keith Milner <kamilner@sslconsult.com> - 2.7.7-4
- smf: fix UPF session leak on UE re-attach; superseded OLD sessions now
  receive a PFCP Session Deletion Request before SMF context is freed,
  preventing orphaned UPF sessions accumulating without bound

* Tue Mar 24 2026 Keith Milner <kamilner@sslconsult.com> - 2.7.7-3
- smf: fix Error Indication handling for dedicated EPC bearers; single-bearer
  removal is now used instead of full PDN session teardown when the affected
  bearer is a dedicated bearer
- pfcp/upf/sgwu: preserve stale FAR hash entry across SGW handovers to prevent
  stale GTP-U Error Indications from triggering runaway session re-establishment
- tests/unit: add pfcp-far regression test binary covering the FAR hash fix

* Tue Mar 17 2026 Keith Milner <kamilner@sslconsult.com> - 2.7.7-2
- Add force_pcscf per-DNN session option to include P-CSCF IPv4 address in
  Create Session Response even when not requested by UE


* Tue Mar 17 2026 Keith Milner <kamilner@sslconsult.com> - 2.7.7-1
- Bump to 2.7.7
- Simplify Release field to a plain revision number; remove branch-aware
  versioning and branchsuffix/buildnum logic

* Mon Mar 09 2026 Keith Milner <kamilner@sslconsult.com> - 2.7.6-2
- Switch to branch-aware versioning: feature-branch builds use 0.1.branchname
  prefix so they sort below the merged release build
- Replace custom autorelease macro with explicit Release field controlled by
  optional branchsuffix define; use build.sh to set it automatically
* Thu Feb 12 2026 Keith Milner <kamilner@sslconsult.com> - 2.7.6-2
- Adds debugging for SMF and small fix for AMBR setting
* Thu Sep 18 2025 Keith Milner <kamilner@sslconsult.com> - 2.7.6-1
- Initial RPM packaging for Open5GS
- Restructured to build individual packages for each service
