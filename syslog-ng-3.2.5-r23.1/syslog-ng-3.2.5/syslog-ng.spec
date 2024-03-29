Summary: Next generation system logging daemon
Name: syslog-ng
Version: 3.2.5
Release: 1
License: GPL
Group: System Environment/Daemons
Source: syslog-ng_%{version}.tar.gz
URL: http://www.balabit.com
Packager: Tamas Pal <folti@balabit.com>
Vendor: BalaBit IT Ltd.
BuildRoot: %{_tmppath}/%{name}-root
# this sucks, but no other way to do it...
BuildRequires: bison, flex, gcc-c++, glib2-devel, pkgconfig, libevtlog-devel, openssl-devel, libnet-devel
Provides: syslog
#BuildConflicts:
#Exclusivearch: i386

%ifarch ppc
%define prefix /opt/freeware
%else
%define prefix /
%endif

%description
 The syslog-ng application is a flexible and highly scalable
 system logging tool. It is often used to manage log messages and implement
 centralized logging, where the aim is to collect the log messages of several
 devices to a single, central log server.

 The main features of syslog-ng include:

   * Support for the BSD (RFC 3164) and IETF (RFC 5424-5428) syslog protocol
     standards
   * Secure log transfer and storage using public-key encryption
   * Reliable log transfer using TCP and TLS
   * Pattern based message classification
   * Direct database access for MSSQL, MySQL, Oracle, PostgreSQL, and SQLite
   * Flow-control
   * Message filtering based on the content and properties of messages
   * Message rewriting
   * IPv4 and IPv6 support
   * Ability to handle high message rates
   * Support for heterogeneous UNIX environments

 For details about syslog-ng, see the syslog-ng homepage at:
    http://www.balabit.com/network-security/syslog-ng/

 The documentation of syslog-ng at:
    http://www.balabit.com/support/documentation/

 Questions, feedback, and bug reports are welcome at the syslog-ng mailing list:
    https://lists.balabit.hu/mailman/listinfo/syslog-ng


%prep
%setup -q -n syslog-ng-%{version}


%build

opt=

# FIXME: set PKG_CONFIG_PATH properly
  ./configure --prefix=%{prefix} --mandir=%{_mandir} --infodir=%{prefix}/share/info \
  --sysconfdir=/etc/syslog-ng --localstatedir=/var/lib/syslog-ng --enable-spoof-source \
	$opt
make

%install
# pre install cleaning. for testing only.
[ $RPM_BUILD_ROOT = / ] || rm -rf $RPM_BUILD_ROOT
make install DESTDIR="$RPM_BUILD_ROOT"
# strip the binaries/ libraries
strip ${RPM_BUILD_ROOT}/%{prefix}/sbin/syslog-ng
./install-sh -d ${RPM_BUILD_ROOT}/etc/syslog-ng
./install-sh -d ${RPM_BUILD_ROOT}/etc/rc.d/init.d

if [ "%{_host_vendor}" = "redhat" ]; then
 ./install-sh -o root -g root -m 0755 contrib/rhel-packaging/syslog-ng.init \
   ${RPM_BUILD_ROOT}/etc/rc.d/init.d/syslog-ng
elif [ "%{_host_vendor}" = "suse" ]; then
 ./install-sh -o root -g root -m 0755 contrib/init.d.SuSE \
   ${RPM_BUILD_ROOT}/etc/rc.d/init.d/syslog-ng
fi

if [ "%{_host_vendor}" = "ibm" ];then
	./install-sh -o root -g bin -m 0644 contrib/aix-packaging/syslog-ng.conf \
	  ${RPM_BUILD_ROOT}/etc/syslog-ng/syslog-ng.conf
elif [ "%{_host_vendor}" = "redhat" ] || [ "%{_host_vendor}" = "suse" ]; then
	install -o root -g root -m 0644 contrib/rhel-packaging/syslog-ng.conf \
	  ${RPM_BUILD_ROOT}/etc/syslog-ng/syslog-ng.conf
	install -D -o root -g root -m 0644 contrib/rhel-packaging/syslog-ng.logrotate \
	  ${RPM_BUILD_ROOT}/etc/logrotate.d/syslog-ng
fi

# install documentation
[ -d "${RPM_BUILD_ROOT}/%{_prefix}/share/doc/syslog-ng-3.2.5" ] || ./install-sh -d "${RPM_BUILD_ROOT}/%{_prefix}/share/doc/syslog-ng-3.2.5"
./install-sh -o root -g root -m 0644 ChangeLog \
  ${RPM_BUILD_ROOT}/%{_prefix}/share/doc/syslog-ng-3.2.5/ChangeLog
./install-sh -o root -g root -m 0644 NEWS \
  ${RPM_BUILD_ROOT}/%{_prefix}/share/doc/syslog-ng-3.2.5/NEWS
./install-sh -o root -g root -m 0644 README \
  ${RPM_BUILD_ROOT}/%{_prefix}/share/doc/syslog-ng-3.2.5/README
./install-sh -o root -g root -m 0644 AUTHORS \
  ${RPM_BUILD_ROOT}/%{_prefix}/share/doc/syslog-ng-3.2.5/AUTHORS
./install-sh -o root -g root -m 0644 COPYING \
  ${RPM_BUILD_ROOT}/%{_prefix}/share/doc/syslog-ng-3.2.5/COPYING

%files
%defattr(-,root,root)
%{prefix}/sbin/syslog-ng
%{prefix}/bin/loggen
%{_mandir}/*
%docdir %{_prefix}/share/doc/syslog-ng-3.2.5
%{_prefix}/share/doc/syslog-ng-3.2.5/*
%config(noreplace) /etc/syslog-ng/syslog-ng.conf
%ifnos aix5.2
/etc/rc.d/init.d/syslog-ng
%config(noreplace) /etc/logrotate.d/syslog-ng
%endif

%post
%ifos aix5.2
mkdir -p /var/lib/syslog-ng || /bin/true
echo "Checking whether the syslog-ng service is already registered... "
if ! /usr/bin/lssrc -s syslogng >/dev/null 2>&1; then
        echo "NO"
        echo "Registering syslog-ng service... "
        if /usr/bin/mkssys -s syslogng -p /opt/freeware/sbin/syslog-ng -u 0 \
          -a '-F -p /etc/syslog-ng.pid' -O -d -Q -S -n 15 -f 9 -E 20 -G ras -w 2 \
          >/dev/null 2>&1; then
                echo "SUCCESSFUL"
        else
                echo "FAILED"
        fi
else
        echo "YES"
fi

echo "Checking whether the syslogd service is registered..."
if /usr/bin/lssrc -s syslogd >/dev/null 2>&1; then
        echo "YES"
        if /usr/bin/lssrc -s syslogd|grep -E "^ syslogd.*active" > /dev/null 2>&1; then
                echo "Stopping the syslogd service..."
                if /usr/bin/stopsrc -s syslogd >/dev/null 2>&1; then
                        echo "SUCCESSFUL"
                else
                        echo "FAILED, continuing anyway"
                fi
        fi
        echo "Disabling syslogd service"
        if /usr/bin/rmssys -s syslogd >/dev/null 2>&1; then
                echo "SUCCESSFUL"
        else
                echo "FAILED"
        fi
else
        echo "NO"
fi

if /usr/bin/lssrc -s syslogng|grep -E "^ syslogng.*active" >/dev/null 2>&1; then
        echo "Stopping syslog-ng"
        /usr/bin/stopsrc -s syslogng
fi
echo "Starting syslog-ng"
/usr/bin/startsrc -s syslogng
#post end
%else
mkdir -p /var/lib/syslog-ng || /bin/true

if [ "%{_host_vendor}" = "redhat" ]; then
  sh /etc/rc.d/init.d/syslog stop || true
elif [ "%{_host_vendor}" = "suse" ]; then
  sh /etc/init.d/syslog stop || true
  if [ -L /etc/init.d/syslog-ng ]; then
    rm -f /etc/init.d/syslog-ng
  fi
  ln -s /etc/rc.d/init.d/syslog-ng /etc/init.d/syslog-ng
fi
chkconfig --del syslog
chkconfig --add syslog-ng
sh /etc/rc.d/init.d/syslog-ng start || exit 0
%endif

%preun
%ifos aix5.2
	if /usr/bin/lssrc -s syslogng >/dev/null 2>&1; then
        	if /usr/bin/lssrc -s syslogng|grep -E "^ syslogng.*active" > /dev/null 2>&1; then
                	echo "Stopping syslog-ng"
	                /usr/bin/stopsrc -s syslogng
	        fi
        	echo "Unregistering syslog-ng"
	        if /usr/bin/rmssys -s syslogng >/dev/null 2>&1; then
        	        echo "SUCCESSFUL"
	        else
        	        echo "FAILED"
	        fi
	fi

# re-enable the standard syslogd subsystem
#subsysname:synonym:cmdargs:path:uid:auditid:standin:standout:standerr:action:multi:contact:svrkey:svrmtype:priority:sig norm:sigforce:display:waittime:grpname:
# syslogd:::/usr/sbin/syslogd:0:0:/dev/console:/dev/console:/dev/console:-O:-Q:-K:0:0:20:0:0:-d:20:ras:

	if ! /usr/bin/lssrc -s syslogd >/dev/null 2>&1; then
        	echo "Registering syslogd service"
	        if /usr/bin/mkssys -s syslogd -p /usr/sbin/syslogd -u 0 -O -Q -K -E 20 -d \
        	  -G ras >/dev/null 2>&1; then
	                echo "SUCCESSFUL"
        	else
                	echo "FAILED"
	        fi
	fi

	if /usr/bin/lssrc -s syslogd >/dev/null 2>&1; then
        	if ! /usr/bin/lssrc -s syslogd | grep -E "^ syslogd.*active" >/dev/null 2>&1; then
	                echo "Starting syslogd"
        	        /usr/bin/startsrc -s syslogd
	        fi
	fi
#preun end
%else
sh /etc/rc.d/init.d/syslog-ng stop || exit 0
if [ "%{_host_vendor}" = "redhat" ]; then
  sh /etc/rc.d/init.d/syslog start || true
elif [ "%{_host_vendor}" = "suse" ]; then
  if [ -L /etc/init.d/syslog-ng ]; then
    rm -f /etc/init.d/syslog-ng
  fi
  sh /etc/init.d/syslog start || true
fi
%endif

%ifnos aix5.2
%check
%endif

%clean
[ $RPM_BUILD_ROOT = / ] || rm -rf $RPM_BUILD_ROOT

%changelog
* Mon Sep 14 2009 Balazs Scheidler <bazsi@balabit.com> 3.2.5-1
- Removed some more Premium Edition related cruft.
* Wed Sep 02 2009 Balazs Scheidler <bazsi@balabit.com> 3.2.5-1
- Removed the references to the documentation and libzlicense-devel
* Wed Apr 04 2007 Tamas Pal <folti@balabit.com> 3.2.5-1
- Added libnet-devel and libzlicense-devel to BuildRequires.
- Changed Packager to Tamas Pal.
- Changed license to BalaBit Proprietary one.
* Wed Nov 22 2006 Tamas Pal <folti@balabit.com>
- Added dependencies glib2-devel and pkgconfig.
- Configfiles /etc/syslog-ng/syslog-ng.conf and /etc/logrotate.d/syslog-ng no
  longer replaced blindly during upgrade.
* Fri Nov 3 2006  Tamas Pal <folti@balabit.com>
- Added SuSE packaging.
- Added AIX 5.2 packaging.
- Added --enable-ssl and --enable-spoof-source configure options
- Now provides syslog
- Added dependencies openssl-devel, libevtlog-devel
* Fri Jun 30 2006 Tamas Pal <folti@balabit.com>
- fixed typo in RHEL config file.
* Mon Mar 27 2006 Balazs Scheidler <bazsi@balabit.com>
- removed postscript version of the documentation
* Fri Sep 9 2005 Sandor Geller <wildy@balabit.com>
- fixed permissions of /etc/rc.d/init.d/syslog-ng
* Thu Jun 30 2005 Sandor Geller <wildy@balabit.com>
- packaging fixes, added logrotate script
* Thu Jun 23 2005 Sandor Geller <wildy@balabit.com>
- added upstream's documentation to the package
* Mon Jun 20 2005 Sandor Geller <wildy@balabit.com>
- initial RPM packaging for RHEL ES

# vim: ts=2 ft=spec
